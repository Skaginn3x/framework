#pragma once

#include <cstdint>
#include <string_view>

#include <tfc/dbus/string_maker.hpp>

namespace tfc::snitch {

enum struct severity_e : std::uint8_t {
  unknown = 0,
  info = 10,
  warning = 20,
  error = 30,
};

namespace dbus {
static constexpr std::string_view service_name{ "Snitch" };
static constexpr std::string_view name{ tfc::dbus::const_dbus_name<service_name> };
static constexpr std::string_view path{ tfc::dbus::const_dbus_path<service_name> };
namespace method {
static constexpr std::string_view set{ "Set" };
static constexpr std::string_view reset{ "Reset" };
}  // namespace method
}  // namespace dbus

}  // namespace tfc::snitch
