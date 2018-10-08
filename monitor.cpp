#include "monitor.h"
#include "remote_server.h"
#include "ThorSerialize/Traits.h"
#include "ThorSerialize/JsonThor.h"
#include "ThorSerialize/SerUtil.h"
#include <sstream>


Monitor::Monitor(const std::string& _id)
    : id(_id) {
  auto& remote_server = getRemoteServer();
  request_number[remote_server.node_address] = 0;
  for (auto& remote_node : remote_server.remote_addresses) {
    request_number[remote_node] = 0;
  }
  if (remote_server.main_node) {
    current_token = Token(remote_server);
  }
  if (remote_server.monitors.find(id) != remote_server.monitors.end()) {
    // Already present
  }
  remote_server.monitors[id] = this;
}

bool Monitor::inCriticalSection() {
  return in_critical_section;
}

void Monitor::Lock() {
  printf("Monitor - Lock\n");
  local_mutex.lock();
  RequestToken();
  in_critical_section = true;
  token_mutex.unlock();
}

void Monitor::Unlock() {
  printf("Monitor - Unlock\n");
  ReleaseToken();
  local_mutex.unlock();
}

void Monitor::RequestToken() {
  printf("Monitor - RequestToken\n");
  token_mutex.lock();
  if (current_token.def == true) {
    //printf("Requesting token now\n");
    request_number[getRemoteServer().node_address]++;
    while (current_token.def == true) { // wait for token
      auto& remote_server = getRemoteServer();
      Message message;
      message.type = MessageType::RequestToken;
      message.monitor_id = id;
      message.sending_server = remote_server.node_address;
      message.data = std::to_string(request_number[remote_server.node_address]);
      for (auto& remote_address : remote_server.remote_addresses) {
        Dispatch dispatch(message, remote_address);
        remote_server.send_queue.push_back(dispatch);
      }
      token_mutex.unlock();
      //printf("Tokens_size: %lu\n", received_tokens.size());
      while (received_tokens.size() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
      }
      //printf("Tokens_size: %lu\n", received_tokens.size());
      current_token = received_tokens[0];
      received_tokens.erase(received_tokens.begin());
      token_mutex.lock();
    }
  }
}

void Monitor::ReleaseToken() {
  printf("Monitor - ReleaseToken\n");
  token_mutex.lock();
  //printf("Releasing token now\n");
  current_token.last_request_number[getRemoteServer().node_address] =
    request_number[getRemoteServer().node_address];
  auto& remote_server = getRemoteServer();
  for (auto& remote_address : remote_server.remote_addresses) {
    if (!(std::find(current_token.token_waiting_queue.begin(),
                    current_token.token_waiting_queue.end(),
                    remote_address) !=
          current_token.token_waiting_queue.end())) {
      if (request_number[remote_address] ==
          current_token.last_request_number[remote_address] + 1) {
        current_token.token_waiting_queue.push_back(remote_address);
      }
    }
  }
  if (current_token.token_waiting_queue.size()) {
    auto dest = current_token.token_waiting_queue[0];
    current_token.token_waiting_queue.erase(current_token.token_waiting_queue.begin());
    SendToken(dest);
  }
  in_critical_section = false;
  token_mutex.unlock();
}

void Monitor::SendToken(const std::string& dest) {
  printf("Monitor - SendToken -> %s\n", dest.c_str());
  std::stringstream json_stream;
  json_stream << ThorsAnvil::Serialize::jsonExport(*this);
  // Insert monitor data to token.
  current_token.data = json_stream.str();
  //printf("Current token data:\n%s\n\n", current_token.data.c_str());

  // New stream.
  json_stream = std::stringstream();
  json_stream << ThorsAnvil::Serialize::jsonExport(current_token);
  std::string token_json = json_stream.str();
  //printf("Current token json \n%s\n\n", token_json.c_str());

  current_token = Token();
  Message message;
  message.type = RespondToken;
  message.monitor_id = id;
  message.sending_server = getRemoteServer().node_address;
  message.data = token_json;
  Dispatch dispatch(message, dest);
  getRemoteServer().send_queue.push_back(dispatch);
}
