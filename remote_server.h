#ifndef REMOTE_SERVER_H_
#define REMOTE_SERVER_H_

#include <string>
#include <vector>
#include <map>
#include "monitor.h"
#include "conditional.h"
#include "sender.h"
#include "receiver.h"
#include "push_socket.h"

RemoteServer& getRemoteServer();

struct RemoteServer {
  RemoteServer();
  int node_address_number;
  std::string node_address;
  std::string node_port;

  // localhost:XXXX adresy wezlow.
  std::vector<std::string> remote_addresses;

  std::map<std::string, PushSocket> remote_server_sockets;
  std::map<std::string, Monitor*> monitors;
  std::map<std::string, Conditional*> conditional_variables;

  std::vector<Dispatch> send_queue;
  bool running = true;
  int working_remote_servers;

  Sender sender_thread;
  Receiver receiver_thread;

  bool main_node = false;
  volatile int sync_barrier;
  
  void CreateRemoteNodes();
  void Start();
  void Finish();
 private:
  void PrepareToStart();
  void Barrier();
  void InformAboutFinish();
};

#endif // REMOTE_SERVER_H_
