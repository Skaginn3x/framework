#include <string>
#include <vector>

#include <ifaddrs.h>

#include <tfc/ec/interfaces.hpp>
#include <tfc/utils/pragmas.hpp>

namespace {
// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wexit-time-destructors)
thread_local std::vector<std::string> interfaces{};
PRAGMA_CLANG_WARNING_POP
// clang-format on
}  // namespace

namespace tfc::global {
auto set_interfaces() -> void {
  struct ifaddrs* addrs;
  getifaddrs(&addrs);

  interfaces.clear();

  for (struct ifaddrs* addr = addrs; addr != nullptr; addr = addr->ifa_next) {
    if (addr->ifa_addr && addr->ifa_addr->sa_family == AF_PACKET) {
      interfaces.emplace_back(addr->ifa_name);
    }
  }
  freeifaddrs(addrs);
}

auto get_interfaces() -> std::vector<std::string> {
  if (interfaces.empty()) {
    set_interfaces();
  }
  return interfaces;
}

}  // namespace tfc::global
