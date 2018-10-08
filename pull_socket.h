#ifndef PULL_SOCKET_H_
#define PULL_SOCKET_H_

#include "zmq.hpp"
#include <string>

struct PullSocket {
  PullSocket();
  void Bind(const std::string& address);
  std::string ReceiveFrameString();
  zmq::context_t ctx = zmq::context_t(1);
  zmq::socket_t socket;
};

#endif // PULL_SOCKET_H_
