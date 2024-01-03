#pragma once

#include <string_view>

#include <fmt/format.h>

#include <tfc/dbus/string_maker.hpp>

namespace tfc::motor::dbus {

namespace detail {
static constexpr std::string_view path_postfix{ "Motors" };
static constexpr std::string_view service{ "Ethercat" };  // needs to match the name in ec.hpp (ethercat exe)
}  // namespace detail

static constexpr std::string_view service_name{ tfc::dbus::const_dbus_name<detail::service> };
static constexpr std::string_view path{ tfc::dbus::const_dbus_path<detail::path_postfix> };

static inline auto make_interface_name(std::string_view implementation_name, std::uint16_t slave_id) -> std::string {
  return tfc::dbus::make_dbus_name(fmt::format("{}_{}", implementation_name, slave_id));
}

namespace method {
static constexpr std::string_view ping{ "Ping" };
static constexpr std::string_view run_at_speedratio{ "RunAtSpeedratio" };
static constexpr std::string_view stop{ "Stop" };
static constexpr std::string_view quick_stop{ "QuickStop" };
static constexpr std::string_view do_homing{ "DoHoming" };
// D-Bus does not support method overloading
// todo do metaprogramming deducing postfix from type, when registering methods
static constexpr std::string_view notify_after_micrometre{ "NotifyAfterMicrometre" };

}  // namespace method

}  // namespace tfc::motor::dbus
