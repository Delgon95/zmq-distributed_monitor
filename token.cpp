#include "token.h"
#include "remote_server.h"

Token::Token(const RemoteServer& remote_server)
    : def(false) {
  last_request_number[remote_server.node_address] = 0;
  for (const auto& remote_addr : remote_server.remote_addresses) {
    last_request_number[remote_addr] = 0;
  }
}
