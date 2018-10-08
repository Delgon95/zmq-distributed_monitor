#include "receiver.h"
#include <string>
#include <sstream>
#include "remote_server.h"
#include "pull_socket.h"
#include "ThorSerialize/Traits.h"
#include "ThorSerialize/JsonThor.h"


Receiver::Receiver(RemoteServer& remote_server)
    : remote_server_(remote_server) {}

void Receiver::Start() {
  thread_ = std::unique_ptr<std::thread>(new std::thread([&](){
        // ZMG pull socket
        auto pull_socket = PullSocket();
        pull_socket.Bind("tcp://*:" + remote_server_.node_port);

        while (remote_server_.running) {
          std::string json_msg = pull_socket.ReceiveFrameString();
          this->ReceiveMessage(json_msg);
        }
        Message message;
        message.type = EndProgram;
        message.sending_server = remote_server_.node_address;
        auto full_message = Dispatch(message, "");
        remote_server_.send_queue.push_back(full_message);
        }));
}

void Receiver::Join() {
  thread_->join();
}

void Receiver::DecrementServers() {
  remote_server_.working_remote_servers--;
  if (remote_server_.working_remote_servers <= 0) {
    remote_server_.running = false;
  }
}

void Receiver::ReceiveMessage(const std::string& json) {
  //printf("Received json_str:\n%s\n\n", json.c_str());
  std::stringstream json_stream(json);
  Message parsed_message;
  json_stream >> ThorsAnvil::Serialize::jsonImport(parsed_message);
  switch (parsed_message.type) {
    case RequestBarrier:
      ReceiveBarrierRequest(parsed_message);
      break;
    case RequestToken:
      ReceiveTokenRequest(parsed_message);
      break;
    case RequestWait:
      ReceiveWaitRequest(parsed_message);
      break;
    case RespondBarrier:
      ReceiveBarrierRespond();
      break;
    case RespondToken:
      ReceiveTokenRespond(parsed_message);
      break;
    case RespondNotifyAll:
      ReceiveNotifyAllRespond(parsed_message);
      break;
    case EndProgram:
      this->DecrementServers();
      break;
  }
}

void Receiver::ReceiveBarrierRequest(const Message& message) {
  printf("Receiver - ReceiveBarrierRequest\n");
  Message response;
  response.type = RespondBarrier;
  Dispatch dispatch(response, message.sending_server);
  remote_server_.send_queue.push_back(dispatch);
}

void Receiver::ReceiveTokenRequest(const Message& message) {
  printf("Receiver - ReceiveTokenRequest\n");
  auto request_number = std::stoi(message.data);
  //printf("request_number: %d\n", request_number);
  //printf("monitor_id: %s\n", message.monitor_id.c_str());
  auto& monitor = remote_server_.monitors[message.monitor_id];
  //printf("Lock\n");
  monitor->token_mutex.lock();
  //printf("Locked\n");
  if (request_number < monitor->request_number[message.sending_server]) {
    //printf("If - return\n");
    monitor->token_mutex.unlock();
    return;
  }

  //printf("request number assign %d\n", request_number);
  monitor->request_number[message.sending_server] = request_number;
  if (monitor->current_token.def == false) {
    //printf("token def false\n");
    if (monitor->inCriticalSection() == false) {
      //printf("monitor not in critical section\n");
      if (monitor->request_number[message.sending_server] ==
          monitor->current_token.last_request_number[message.sending_server] + 1) {
        //printf("Send Token\n");
        monitor->SendToken(message.sending_server);
      }
    }
  }
  //printf("token unlock\n");
  monitor->token_mutex.unlock();
}

void Receiver::ReceiveWaitRequest(const Message& message) {
  printf("Receiver - ReceiveWaitRequest\n");
  auto& conditional = remote_server_.conditional_variables[message.data];
  //printf("Lock\n");
  conditional->condition_mutex.lock();
  //printf("Locked\n");
  if (!(std::find(conditional->remote_nodes_waiting_queue.begin(),
                 conditional->remote_nodes_waiting_queue.end(),
                 message.sending_server) !=
      conditional->remote_nodes_waiting_queue.end())) {
    //printf("If statement\n");
    conditional->remote_nodes_waiting_queue.push_back(message.sending_server);
  }
  //printf("Unlock\n");
  conditional->condition_mutex.unlock();

}

void Receiver::ReceiveBarrierRespond() {
  printf("Receiver - ReceiveBarrierRespond\n");
  --remote_server_.sync_barrier;
}

void Receiver::ReceiveTokenRespond(const Message& message) {
  printf("Receiver - ReceiveTokenRespond\n");
  Token token;
  std::stringstream json_stream(message.data);
  //printf("Message data:\n%s\n\n", message.data.c_str());
  json_stream >> ThorsAnvil::Serialize::jsonImport(token);
  token.def = false;

  auto& monitor = remote_server_.monitors[message.monitor_id];
  //printf("Wait for token\n");
  monitor->token_mutex.lock();

  auto monitor_id = monitor->id;
  json_stream = std::stringstream(token.data);
  //printf("Token data received:\n%s\n\n", token.data.c_str());
  monitor->products_.clear();
  json_stream >> ThorsAnvil::Serialize::jsonImport(*monitor);
  monitor->id = monitor_id;
  //printf("Token data monitor: products %lu\n", monitor->products_.size());
  //for (auto a : monitor->products_) {
  //  printf("%d ", a);
  //}
  //printf("\n");

  monitor->received_tokens.push_back(token);
  //printf("Received tokens %lu\n", monitor->received_tokens.size());
  monitor->token_mutex.unlock();
}

void Receiver::ReceiveNotifyAllRespond(const Message& message) {
  printf("NotifyAll Respond\n");
  auto& conditional = remote_server_.conditional_variables[message.data];
  conditional->condition_mutex.lock();
  if (!(std::find(conditional->remote_nodes_waiting_queue.begin(),
                  conditional->remote_nodes_waiting_queue.end(),
                  remote_server_.node_address) !=
      conditional->remote_nodes_waiting_queue.end())) {
    conditional->condition_mutex.unlock();
    return;
  }
  auto id = std::find(conditional->remote_nodes_waiting_queue.begin(),
                        conditional->remote_nodes_waiting_queue.end(),
                        remote_server_.node_address);
  conditional->remote_nodes_waiting_queue.erase(id);
  for (auto& queue : conditional->conditional_queues) {
    queue->push_back(message.sending_server);
  }
  conditional->conditional_queues.clear();
  conditional->condition_mutex.unlock();
}
