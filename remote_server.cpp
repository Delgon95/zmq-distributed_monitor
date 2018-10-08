#include "remote_server.h"
#include <string>
#include <vector>
#include <thread>
#include <chrono>

namespace {

std::vector<std::string> SplitString(const std::string& str,
                                     const std::string& delimiter) {
  std::vector<std::string> res;
  size_t pos = 0;
  size_t newpos;
  while ((newpos = str.find(delimiter, pos)) != std::string::npos) {
    res.push_back(str.substr(pos, newpos - pos));
    pos = newpos + delimiter.size();
  }

  res.push_back(str.substr(pos, newpos - pos));
  return res;
}

} // namespace

RemoteServer& getRemoteServer() {
  static auto* remote_server = new RemoteServer();
  return *remote_server;
}

RemoteServer::RemoteServer()
    : sender_thread(Sender(*this)), receiver_thread(Receiver(*this)) {
}

void RemoteServer::CreateRemoteNodes() {
  if (node_address_number >= static_cast<int>(remote_addresses.size())) {
    printf ("Wrong node number\n");
    return;
  }
  node_address = remote_addresses[static_cast<size_t>(this->node_address_number)];
  printf("My address: %s\n", node_address.c_str());
  if (node_address_number == 0) {
    printf("Main node\n");
    main_node = true;
  }

  working_remote_servers = static_cast<int>(remote_addresses.size());
  remote_addresses.erase(remote_addresses.begin() + node_address_number);

  node_port = SplitString(node_address, ":")[1];

  for (const auto& remote_server : remote_addresses) {
    printf("Pushing PushSocket for address: %s\n", remote_server.c_str());
    remote_server_sockets[remote_server] = PushSocket();
    remote_server_sockets[remote_server].Connect("tcp://" + remote_server);
  }
}

void RemoteServer::Finish() {
  auto& remote_server = getRemoteServer();
  remote_server.InformAboutFinish();
  remote_server.sender_thread.Join();
  remote_server.receiver_thread.Join();
}

void RemoteServer::InformAboutFinish() {
  Message message;
  message.type = EndProgram;
  message.sending_server = this->node_address;
  Dispatch dispatch;
  dispatch.message = message;
}

void RemoteServer::Barrier() {
  Message message;
  message.type = RespondBarrier;
  message.sending_server = node_address;
  for (const auto& remote_address : remote_addresses) {
    send_queue.push_back(Dispatch(message, remote_address));
  }
  printf("Waiting for other nodes - barrier\n");
  printf("Init wait: %d | %lu\n", sync_barrier, remote_addresses.size());
  while (sync_barrier != 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  printf("Barried ended!\n");
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void RemoteServer::Start() {
  sync_barrier = static_cast<int>(this->remote_addresses.size());
  receiver_thread.Start();
  sender_thread.Start();

  Barrier();
  printf("Remote server finished initialization\n");
}
