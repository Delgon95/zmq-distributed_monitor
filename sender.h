#ifndef SENDER_H_
#define SENDER_H_

#include "message.h"
#include <memory>
#include <thread>

struct RemoteServer;

class Sender {
 public:
  Sender(RemoteServer& remote_server);
  void Start();
  void Join();
  void SendMessage(const Dispatch& dispatch);
  void DecrementServers();
 private:
  RemoteServer& remote_server_;
  std::unique_ptr<std::thread> thread_;
};

#endif // SENDER_H_
