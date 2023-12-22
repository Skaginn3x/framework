#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <net/if.h>
#include <glaze/core/common.hpp>

#include <tfc/utils/json_schema.hpp>

// #include <signal_names.hpp>

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
  static void op(auto& s, auto& def) noexcept {
    if (!s.oneOf.has_value()) {
      s.oneOf = std::vector<tfc::json::detail::schematic>{};
    }


    // std::vector<std::string> interfaces{ "one", "two" };

    struct if_nameindex* if_ni;
    struct if_nameindex* i;

    if_ni = if_nameindex();
    if (if_ni == nullptr) {
      // Handle error or perform fallback logic
      return;
    }

    std::vector<std::string> interfaces{};

    // clang-format off
    PRAGMA_CLANG_WARNING_PUSH_OFF(-Wunsafe-buffer-usage)
    for (i = if_ni; i->if_index != 0 || i->if_name != nullptr; i++) {
      interfaces.push_back(i->if_name);
      //  s.oneOf.value().push_back(tfc::json::detail::schematic{
      //      .attributes{ tfc::json::schema{ .title = i->if_name, .description = i->if_name, .constant = i->if_name } }
      //  });
    }
    PRAGMA_CLANG_WARNING_POP
    // clang-format on

    if_freenameindex(if_ni);  // Free the allocated memory

    for (auto& interface : interfaces) {
      s.oneOf.value().emplace_back(tfc::json::detail::schematic{
          .attributes{ tfc::json::schema{ .title = interface, .description = interface, .constant = interface } } });
      // def[0]->second.oneOf.value().emplace_back(tfc::json::detail::schematic{
          // .attributes{ tfc::json::schema{ .title = interface, .description = interface, .constant = interface } } });
    }

    //  for (auto const& interface : interfaces) {
    //    s.oneOf.value().push_back(tfc::json::detail::schematic{
    //        .attributes{ tfc::json::schema{ .title = interface, .description = interface, .constant = interface } } });
    //  }
  }
};

namespace tfc::ec::config {

struct network_interfaces {
  network_interface interfaces{};
  // std::vector<network_interface> interfaces{};

  struct glaze {
    static constexpr auto value{ glz::object("interfaces", &network_interfaces::interfaces, "Network interfaces") };
    static constexpr std::string_view name{ "network_interfaces" };
  };
};

}  // namespace tfc::ec::config
