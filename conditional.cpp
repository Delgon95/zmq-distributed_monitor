#include "conditional.h"
#include "message.h"
#include "remote_server.h"



Conditional::Conditional(const std::string& _id, Monitor* _monitors)
    : id(_id) {

  getRemoteServer().conditional_variables[id] = this;
  monitors.push_back(_monitors);
}

void Conditional::Wait() {
  printf("Conditional - Wait\n");
  condition_mutex.lock();
  //printf("Wait - locked\n");
  std::vector<std::string> for_signal;
  conditional_queues.push_back(&for_signal);
  auto& remote_server = getRemoteServer();
  if (!(std::find(remote_nodes_waiting_queue.begin(),
                  remote_nodes_waiting_queue.end(),
                  remote_server.node_address) !=
      remote_nodes_waiting_queue.end())) {
    //printf("Wait - ifstatement\n");
    remote_nodes_waiting_queue.push_back(remote_server.node_address);
  }
  Message message;
  message.type = RequestWait;
  message.data = id;
  message.sending_server = remote_server.node_address;

  //printf("Wait - for\n");
  for (const auto& remote_addr : remote_server.remote_addresses) {
    Dispatch remote_dispatch = Dispatch(message, remote_addr);
    remote_server.send_queue.push_back(remote_dispatch);
  }

  //printf("Unlock monitors\n");
  for (int i = static_cast<int>(monitors.size() - 1); i >= 0; --i) {
    monitors[static_cast<size_t>(i)]->Unlock();
  }
  //printf("Unlock conditional\n");
  condition_mutex.unlock();
  //printf("Wait for notification size: %lu\n", for_signal.size());
  while (for_signal.size() == 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
  }
  //printf("Notification size: %lu\n", for_signal.size());
  std::string notified_by = for_signal[0];
  for_signal.erase(for_signal.begin());
  //printf("Notification received by %s\n", notified_by.c_str());
  for (auto& monitor : monitors) {
    monitor->Lock();
  }
}

void Conditional::NotifyAll() {
  printf("Conditional - NotifyAll\n");
  condition_mutex.lock();
  Message message;
  message.type = RespondNotifyAll;
  message.data = id;
  message.sending_server = getRemoteServer().node_address;
  while (remote_nodes_waiting_queue.size() > 0) {
    if (remote_nodes_waiting_queue[0] == getRemoteServer().node_address) {
      for (auto& queue : conditional_queues) {
        queue->push_back(getRemoteServer().node_address);
      }
      conditional_queues.clear();
    } else {
      auto remote_addr = remote_nodes_waiting_queue[0];
      auto remote_dispatch = Dispatch(message, remote_addr);
      getRemoteServer().send_queue.push_back(remote_dispatch);
    }
    remote_nodes_waiting_queue.erase(remote_nodes_waiting_queue.begin());
  }
  condition_mutex.unlock();
}
