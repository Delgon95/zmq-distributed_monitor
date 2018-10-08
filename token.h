#ifndef TOKEN_H_
#define TOKEN_H_

#include <map>
#include <string>
#include <vector>
#include "ThorSerialize/Traits.h"
#include "ThorSerialize/JsonThor.h"
#include "ThorSerialize/SerUtil.h"

struct RemoteServer;

struct Token {
  Token() = default;
  Token(const RemoteServer& remote_server);
  volatile bool def = true;
  std::vector<std::string> token_waiting_queue;
  std::map<std::string, int> last_request_number;
  std::string data;
};

ThorsAnvil_MakeTrait(Token, token_waiting_queue, last_request_number, data);

#endif // TOKEN_H_
