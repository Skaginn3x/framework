#pragma once

#include <cstdint>
#include <string_view>

#include <magic_enum.hpp>

#include <tfc/dbus/string_maker.hpp>
#include <tfc/stx/basic_fixed_string.hpp>
#include <tfc/stx/string_view_join.hpp>
#include <tfc/stx/to_tuple.hpp>

#define EXPORT __attribute__((visibility("default")))

namespace tfc::operation {

enum struct mode_e : std::uint8_t {
  unknown = 0,
  stopped = 1,
  starting = 2,
  running = 3,
  stopping = 4,
  cleaning = 5,
  emergency = 6,
  fault = 7,
  maintenance = 8,
};

[[nodiscard]] inline constexpr auto mode_e_str(mode_e enum_value) {
  return magic_enum::enum_name(enum_value);
}

namespace dbus {
static constexpr std::string_view service_name{ "operation_mode" };
static constexpr std::string_view name{ tfc::dbus::const_dbus_name<service_name> };
static constexpr std::string_view path{ tfc::dbus::const_dbus_path<service_name> };
namespace method {
static constexpr std::string_view set_mode{ "set_mode" };
}
namespace signal {
static constexpr std::string_view update{ "update" };
}  // namespace signal
}  // namespace dbus

struct update_message {
  mode_e new_mode{ mode_e::unknown };
  mode_e old_mode{ mode_e::unknown };

  static constexpr auto dbus_reflection{ [](auto&& self) {
    return tfc::stx::to_tuple(std::forward<decltype(self)>(self));
  } };
};

}  // namespace tfc::operation
