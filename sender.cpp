#include "sender.h"
#include "remote_server.h"
#include <string>
#include <sstream>
#include "ThorSerialize/Traits.h"
#include "ThorSerialize/JsonThor.h"
#include "ThorSerialize/SerUtil.h"

Sender::Sender(RemoteServer& remote_server)
    : remote_server_(remote_server) {}

void Sender::Start() {
  thread_ = std::unique_ptr<std::thread>(new std::thread([&](){
        while (remote_server_.running) {
          //printf("Waiting for message to send\n");
          while (remote_server_.send_queue.size() == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
          }
          auto message_to_send = remote_server_.send_queue[0];
          //printf("Sending message type: %d to: %s\n",
          //    message_to_send.message.type,
          //    message_to_send.destination.c_str());
          remote_server_.send_queue.erase(remote_server_.send_queue.begin());
          if (message_to_send.message.type == EndProgram) {
            this->DecrementServers();
            for (const auto& remote_addrss : remote_server_.remote_addresses) {
              message_to_send.destination = remote_addrss;
              this->SendMessage(message_to_send);
            }
            continue;
          }
          this->SendMessage(message_to_send);
        }
        }));
}

void Sender::DecrementServers() {
  remote_server_.working_remote_servers--;
  if (remote_server_.working_remote_servers <= 0) {
    remote_server_.running = false;
  }
}

void Sender::Join() {
  thread_->join();
}

namespace {

std::string TypeToString(MessageType type) {
  switch (type) {
    case RequestBarrier:
      return "RequestBarrier";
    case RequestToken:
      return "RequestToken";
    case RequestWait:
      return "RequestWait";
    case RespondBarrier:
      return "RespondBarrier";
    case RespondToken:
      return "RespondToken";
    case RespondNotifyAll:
      return "RespondNotifyAll";
    case EndProgram:
      return "EndProgram";
  }
  return "\n\t\t\t! Unknown Type !\n\n";
}

} // namespace

void Sender::SendMessage(const Dispatch& dispatch) {
  printf("Sender - SendMessage - %s -> %s\n",
      TypeToString(dispatch.message.type).c_str(), dispatch.destination.c_str());
  std::stringstream json_stream;
  json_stream << ThorsAnvil::Serialize::jsonExport(dispatch.message);
  std::string json_str = json_stream.str();
  //printf("Sending json string:\n%s\n\n", json_str.c_str());
  remote_server_.remote_server_sockets[dispatch.destination].SendFrame(json_str);
}
