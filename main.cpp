#include "monitor.h"
#include "remote_server.h"
#include "conditional.h"

#include <vector>
#include <thread>
#include <chrono>
#include "ThorSerialize/Traits.h"
#include "ThorSerialize/JsonThor.h"
#include "ThorSerialize/SerUtil.h"

namespace {

std::string ValuesToString(const std::vector<int>& vals) {
  std::string res;
  for (auto val : vals) {
    res += std::to_string(val) + " ";
  }
  return res;
}

}

[[ noreturn ]] void Producent(Conditional& products_empty,
               Conditional& products_full,
               ProducentConsument& monitor) {
  while (1) {
    printf("\t\tMain Producent - LOCK\n");
    monitor.Lock();
    while (monitor.products_.size() == monitor.max_products_) {
      printf("\t\tMain Producent - WAIT products: %lu/%lu\n", monitor.products_.size(), monitor.max_products_);
      products_full.Wait();
    }
    printf("\t\tMain Producent - Current Products: %s\n", ValuesToString(monitor.products_).c_str());
    printf("\t\tMain Producent - Producing Product: %d\n", ++monitor.last_product_);
    monitor.products_.push_back(monitor.last_product_);
    printf("\t\tMain Producent - Current Products: %s\n", ValuesToString(monitor.products_).c_str());
    printf("\t\tMain Producent - Notify all\n");
    products_empty.NotifyAll();
    printf("\t\tMain Producent - Unlock\n");
    monitor.Unlock();
    printf("\t\tMain Producent - Waiting for 1s\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
  }
}

[[ noreturn ]] void Consument(Conditional& products_empty,
               Conditional& products_full,
               ProducentConsument& monitor) {
  while (1) {
    printf("\t\tMain Consument - LOCK\n");
    monitor.Lock();
    while (monitor.products_.size() == 0) {
      printf("\t\tMain Consument - WAIT products: %lu/%lu\n", monitor.products_.size(), monitor.max_products_);
      products_empty.Wait();
    }
    printf("\t\tMain Consument - Current Products: %s\n", ValuesToString(monitor.products_).c_str());
    auto taken_value = monitor.products_[0];
    printf("\t\tMain Consument - Consumed Product: %d\n", taken_value);
    monitor.products_.erase(monitor.products_.begin());
    printf("\t\tMain Consument - Current Products: %s\n", ValuesToString(monitor.products_).c_str());
    printf("\t\tMain Consument - Notify all\n");
    products_full.NotifyAll();
    printf("\t\tMain Consument - Unlock\n");
    monitor.Unlock();
    printf("\t\tMain Consument - Waiting for 2s\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }
}


int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Invalid number of arguments.\n");
    printf("Usage: ./program node_number\n");
    printf("node_number: Run with selected node - 0, 1, 2\n");
  }
  int node_number = std::stoi(argv[1]);
  if (node_number > 2 || node_number < 0) {
    printf("Invalid node_number parameter.\n");
    return 1;
  }
  // Initiate RemoteServer();
  auto& remote_server = getRemoteServer();
  std::vector<std::string> addresses = {"localhost:5555",
                                        "localhost:6666",
                                        "localhost:7777"};

  remote_server.node_address_number = node_number;
  remote_server.remote_addresses.push_back(addresses[0]);
  remote_server.remote_addresses.push_back(addresses[1]);
  remote_server.remote_addresses.push_back(addresses[2]);

  remote_server.CreateRemoteNodes();

  ProducentConsument* prod_cons = new ProducentConsument(5, "monitor");
  Conditional products_empty("EmptyProducts", prod_cons);
  Conditional products_full("FullProducts", prod_cons);

  remote_server.Start();
  if (node_number == 0) {
    Producent(products_empty, products_full, *prod_cons);
  } else {
    Consument(products_empty, products_full, *prod_cons);
  }
  remote_server.Finish();
  return 0;
}
