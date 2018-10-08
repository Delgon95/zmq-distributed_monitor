#include "pull_socket.h"

PullSocket::PullSocket()
    : socket(zmq::socket_t(ctx, ZMQ_PULL)) {}

void PullSocket::Bind(const std::string& address) {
  socket.bind(address.c_str());
}

std::string PullSocket::ReceiveFrameString() {
  zmq::message_t msg;
  socket.recv(&msg);
  return std::string(static_cast<char*>(msg.data()), msg.size());
}
