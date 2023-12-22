#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <ifaddrs.h>
#include <glaze/core/common.hpp>
#include <tfc/utils/json_schema.hpp>

namespace tfc::ec::config {

struct network_interface {
  std::string value{};

  struct glaze {
    static constexpr auto value = &network_interface::value;
    static constexpr std::string_view name{ "network_interface" };
  };
};
}  // namespace tfc::ec::config

template <>
struct tfc::json::detail::to_json_schema<tfc::ec::config::network_interface> {
  template <auto Opts>
  static void op(auto& s, auto&) noexcept {
    if (!s.oneOf.has_value()) {
      s.oneOf = std::vector<tfc::json::detail::schematic>{};
    }

    struct ifaddrs* addrs;
    getifaddrs(&addrs);

    for (struct ifaddrs* addr = addrs; addr != nullptr; addr = addr->ifa_next) {
      if (addr->ifa_addr && addr->ifa_addr->sa_family == AF_PACKET) {
        s.oneOf.value().push_back(tfc::json::detail::schematic{ .attributes{
            tfc::json::schema{ .title = addr->ifa_name, .description = addr->ifa_name, .constant = addr->ifa_name } } });
      }
    }

    freeifaddrs(addrs);
  }
};

namespace tfc::ec::config {

struct network_interfaces {
  network_interface interfaces{};

  struct glaze {
    static constexpr auto value{ glz::object("interfaces", &network_interfaces::interfaces, "Network interfaces") };
    static constexpr std::string_view name{ "network_interfaces" };
  };
};

}  // namespace tfc::ec::config
