#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <glaze/core/common.hpp>
#include <tfc/utils/json_schema.hpp>
#include <tfc/utils/pragmas.hpp>

#include <tfc/ec/interfaces.hpp>

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
    s.oneOf = std::vector<tfc::json::detail::schematic>{};

    PRAGMA_CLANG_WARNING_PUSH_OFF(-Wexit - time - destructors)
    PRAGMA_CLANG_WARNING_PUSH_OFF(-Wglobal - constructors)
    thread_local auto interfaces{ tfc::global::get_interfaces() };
    PRAGMA_CLANG_WARNING_POP
    PRAGMA_CLANG_WARNING_POP

    for (auto const& interface : interfaces) {
      s.oneOf->emplace_back(tfc::json::detail::schematic{
          .attributes{ tfc::json::schema{ .title = interface, .description = interface, .constant = interface } } });
    }
  }
};

namespace tfc::ec::config {
struct bus {
  network_interface primary_interface{ tfc::global::get_interfaces()[0] };
  struct glaze {
    static constexpr auto value{ glz::object("primary_interface", &bus::primary_interface, "Primary interface") };
    static constexpr std::string_view name{ "config::bus" };
  };
};

}  // namespace tfc::ec::config
