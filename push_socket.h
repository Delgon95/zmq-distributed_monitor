#ifndef PUSH_SOCKET_H_
#define PUSH_SOCKET_H_

#include "zmq.hpp"
#include <string>

struct PushSocket {
  PushSocket();
  void Connect(const std::string& address);
  void SendFrame(const std::string& frame);

  zmq::context_t ctx = zmq::context_t(1);
  zmq::socket_t socket;
};

#endif // PUSH_SOCKET_H_
