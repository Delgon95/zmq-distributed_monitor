#include "push_socket.h"

PushSocket::PushSocket()
    : socket(zmq::socket_t(ctx, ZMQ_PUSH)) {
}

void PushSocket::Connect(const std::string& address) {
  socket.connect(address.c_str());
}

void PushSocket::SendFrame(const std::string& frame) {
  zmq::message_t msg(frame.size());
  memcpy ((void*)msg.data(), frame.c_str(), frame.size());
  socket.send(msg);
}
