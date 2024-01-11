#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <glaze/core/common.hpp>

#include <tfc/confman/observable.hpp>
#include <tfc/ec/interfaces.hpp>
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
    s.oneOf = std::vector<tfc::json::detail::schematic>{};

    auto interfaces{ tfc::global::get_interfaces() };

    for (auto const& interface : interfaces) {
      s.oneOf->emplace_back(tfc::json::detail::schematic{
          .attributes{ tfc::json::schema{ .title = interface, .description = interface, .constant = interface } } });
    }
  }
};

namespace tfc::ec::config {
struct ethercat {
  network_interface primary_interface{ tfc::global::get_interfaces()[0] };
  confman::observable<std::optional<std::size_t>> required_slave_count{ 0 };
  struct glaze {
    // clang-format off
    static constexpr auto value{ glz::object("primary_interface", &ethercat::primary_interface, "Primary interface",
                                             "required_slave_count", &ethercat::required_slave_count, "Required slave count") };
    // clang-format on
    static constexpr std::string_view name{ "config::ethercat" };
  };
};

}  // namespace tfc::ec::config
