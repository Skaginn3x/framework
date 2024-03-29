#include <string>
#include <vector>

#include <ifaddrs.h>

#include <tfc/ec/common.hpp>
#include <tfc/utils/pragmas.hpp>

namespace {
// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wexit-time-destructors)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wglobal-constructors)
thread_local std::vector<std::string> interfaces{};
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
// clang-format on
}  // namespace

namespace tfc::ec::common {

auto get_interfaces() -> std::vector<std::string> const& {
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

}  // namespace tfc::ec::common
