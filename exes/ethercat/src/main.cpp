// /** \file
//  * \brief Example code for running context_t
//  *
//  * Usage: context_example --iface IFACE --id <some_id> --stdout --log-level <log_level>
//  * IFNAME is the NIC interface name, e.g. 'eth0'
//  *
//  */
//
// #include <ranges>
//
// #include <boost/asio/signal_set.hpp>
// #include <boost/program_options.hpp>
//
// #include "tfc/ec.hpp"
// #include "tfc/progbase.hpp"
//
// auto main(int argc, char* argv[]) -> int {
//   auto prog_desc{ tfc::base::default_description() };
//   tfc::base::init(argc, argv, prog_desc);
//
//   boost::asio::io_context io_ctx;
//   tfc::ec::context_t ctx(io_ctx);
//
//   ctx.async_start();
//
//   io_ctx.run();
//
//   return 0;
// }

#include <ifaddrs.h>  // for getifaddrs, freeifaddrs
#include <cstring>    // for strdup, free
#include <iostream>
#include <set>

int main() {
  struct ifaddrs* ifaddr;
  struct ifaddrs* ifa;

  std::set<std::string> interface_names;

  if (getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs");
    return -1;
  }

  for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == nullptr) {
      continue;
    }
    interface_names.insert(ifa->ifa_name);
  }

  for (const auto& name : interface_names) {
    std::cout << name.data() << std::endl;
  }
}
