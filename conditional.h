#ifndef CONDITIONAL_H_
#define CONDITIONAL_H_

#include <string>
#include <vector>
#include <mutex>
#include "monitor.h"

struct Conditional {
  Conditional(const std::string& _id, Monitor* _monitors);
  void Wait();
  void NotifyAll();

  std::string id;
  std::vector<Monitor*> monitors;
  std::vector<std::vector<std::string>*> conditional_queues;
  std::vector<std::string> remote_nodes_waiting_queue;
  std::mutex condition_mutex;
};

#endif // CONDITIONAL_H_
