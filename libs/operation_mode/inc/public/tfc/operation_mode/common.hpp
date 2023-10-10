#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#include <tfc/dbus/string_maker.hpp>
#include <tfc/stx/basic_fixed_string.hpp>
#include <tfc/stx/string_view_join.hpp>
#include <tfc/stx/to_tuple.hpp>

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

[[nodiscard]] auto enum_name(mode_e enum_value) -> std::string_view;
[[nodiscard]] auto enum_cast(std::string_view enum_name) -> std::optional<mode_e>;

namespace dbus {
// Please note that this needs to match the generated service name of the operation mode daemon
static constexpr std::string_view service_default{ "tfc.Operations.def" };
static constexpr std::string_view service_name{ tfc::dbus::const_dbus_name<service_default> };
static constexpr std::string_view interface_name{ "OperationMode" };
static constexpr std::string_view name{ tfc::dbus::const_dbus_name<interface_name> };
static constexpr std::string_view path{ tfc::dbus::const_dbus_path<interface_name> };
namespace method {
static constexpr std::string_view set_mode{ "SetMode" };
}
namespace signal {
static constexpr std::string_view update{ "Update" };
}  // namespace signal
namespace property {
static constexpr std::string_view mode{ "Mode" };
}
}  // namespace dbus

struct update_message {
  mode_e new_mode{ mode_e::unknown };
  mode_e old_mode{ mode_e::unknown };

  static constexpr auto dbus_reflection{ [](auto&& self) {
    return tfc::stx::to_tuple(std::forward<decltype(self)>(self));
  } };
};

}  // namespace tfc::operation
