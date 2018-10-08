#ifndef MONITOR_H_
#define MONITOR_H_

#include <string>
#include <map>
#include <mutex>
#include <vector>
#include "token.h"
#include "sender.h"
#include "receiver.h"
#include "push_socket.h"
#include "ThorSerialize/Traits.h"
#include "ThorSerialize/JsonThor.h"
#include "ThorSerialize/SerUtil.h"

class Monitor {
 public:
  Monitor() = default;
  virtual ~Monitor() = default;
  Monitor(const std::string& _id);
  void Lock();
  void Unlock();
  void RequestToken();
  void ReleaseToken();
  void SendToken(const std::string& dest);

  Token current_token;
  std::vector<Token> received_tokens;
  bool inCriticalSection();
  std::map<std::string, int> request_number;
  std::mutex token_mutex;
  std::mutex local_mutex;
  std::string id;

  // problems with derived.
  std::vector<int> products_;

  friend class ThorsAnvil::Serialize::Traits<Monitor>;
  //ThorsAnvil_PolyMorphicSerializer(Monitor);
  bool in_critical_section = false;
};

class ProducentConsument : public Monitor {
 public:
  //std::vector<int> products_;
  size_t max_products_;
  int last_product_ = 0;
  ProducentConsument(size_t max_products,
                     const std::string& monitor_id)
    : Monitor(monitor_id), max_products_(max_products) {}
  friend class ThorsAnvil::Serialize::Traits<ProducentConsument>;
  //ThorsAnvil_PolyMorphicSerializer(ProducentConsument);
};

ThorsAnvil_MakeTrait(Monitor, id, products_);
ThorsAnvil_ExpandTrait(Monitor, ProducentConsument, max_products_);

#endif // MONITOR_H_
