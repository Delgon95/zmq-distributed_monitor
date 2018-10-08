#ifndef RECEIVER_H_
#define RECEIVER_H_

#include "message.h"
#include <memory>
#include <thread>

struct RemoteServer;

class Receiver {
 public:
  Receiver(RemoteServer& remote_server);
  void Start();
  void Join();
  void DecrementServers();
  void ReceiveMessage(const std::string& json);
 private:
  // Messages handlers.
  void ReceiveBarrierRequest(const Message& message);
  void ReceiveTokenRequest(const Message& message);
  void ReceiveWaitRequest(const Message& message);
  void ReceiveBarrierRespond();
  void ReceiveTokenRespond(const Message& message);
  void ReceiveNotifyAllRespond(const Message& message);
  RemoteServer& remote_server_;
  std::unique_ptr<std::thread> thread_;
};

#endif // RECEIVER_H_
