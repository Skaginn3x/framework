#include <string>
#include <vector>

#include <ifaddrs.h>

#include <tfc/ec/interfaces.hpp>
#include <tfc/utils/pragmas.hpp>

namespace tfc::global {

auto get_interfaces() -> std::vector<std::string> {
  std::vector<std::string> interfaces{};

  struct ifaddrs* addrs;
  getifaddrs(&addrs);

  interfaces.clear();

  for (struct ifaddrs* addr = addrs; addr != nullptr; addr = addr->ifa_next) {
    if (addr->ifa_addr && addr->ifa_addr->sa_family == AF_PACKET) {
      interfaces.emplace_back(addr->ifa_name);
    }
  }
  freeifaddrs(addrs);

  return interfaces;
}

}  // namespace tfc::global
