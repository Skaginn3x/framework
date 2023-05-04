#pragma once

#include <cstdint>
#include <string_view>

#include <boost/mp.hpp>
#include <magic_enum.hpp>

#include <tfc/stx/basic_fixed_string.hpp>
#include <tfc/stx/string_view_join.hpp>
#include <tfc/utils/dbus.hpp>

namespace tfc::operation {

enum struct mode_e : std::uint8_t {
  unknown = 0,
  stopped = 1,
  running = 2,
  specialized_running_1 = 3,
  specialized_running_2 = 4,
  specialized_running_3 = 5,
  fault = 6,
  cleaning = 7,
  emergency = 8,
  maintenance = 9,
};

[[nodiscard]] inline constexpr auto mode_e_str(mode_e enum_value) {
  return magic_enum::enum_name(enum_value);
}

namespace dbus {
static constexpr std::string_view service_name{ "operation_mode" };
static constexpr std::string_view name{ tfc::dbus::const_dbus_name<service_name> };
static constexpr std::string_view path{ tfc::dbus::const_dbus_path<service_name> };
namespace signal {
static constexpr std::string_view update{ "update" };
}  // namespace signal
}  // namespace dbus

struct update_message {
  mode_e new_mode{ mode_e::unknown };
  mode_e old_mode{ mode_e::unknown };

  static constexpr auto dbus_reflection{ [](auto&& self) { return boost::mp::reflection::to_tuple(self); } };
};

}  // namespace tfc::operation

namespace sdbusplus::message::types::details {

namespace concepts {

template <typename struct_t>
concept dbus_reflectable = requires {
  struct_t::dbus_reflection;
  requires std::invocable<decltype(struct_t::dbus_reflection), struct_t&&>;
};

}

template <typename value_t, typename enable_t>
struct type_id;

template <concepts::dbus_reflectable struct_t>
struct type_id<struct_t, void> {
  static constexpr auto value{ type_id<decltype(struct_t::dbus_reflection(struct_t{})), void>::value };
};

}  // namespace sdbusplus::message::types::details

namespace sdbusplus::message::details {

template <typename value_t>
struct convert_from_string;

template <typename value_t>
struct convert_to_string;

template <>
struct convert_from_string<tfc::operation::mode_e> {
  static auto op(const std::string& mode_str) noexcept -> std::optional<tfc::operation::mode_e> {
    return magic_enum::enum_cast<tfc::operation::mode_e>(mode_str);
  }
};

template <>
struct convert_to_string<tfc::operation::mode_e> {
  static auto op(tfc::operation::mode_e mode) -> std::string {
    return std::string{ magic_enum::enum_name(mode) };
  }
};

}  // namespace sdbusplus::message::details

// template <>
// struct glz::meta<tfc::operation::mode_e> {
//   using enum tfc::operation::mode_e;
//   // clang-format off
//   static constexpr auto value{ glz::enumerate(
//       "unknown", unknown,
//       "stopped", stopped,
//       "running", running,
//       "specialized_running_1", specialized_running_1,
//       "specialized_running_2", specialized_running_2,
//       "specialized_running_3", specialized_running_3,
//       "fault", fault,
//       "cleaning", cleaning,
//       "emergency", emergency,
//       "maintenance", maintenance
//   ) };
//   // clang-format on
//   static constexpr std::string_view name{ "tfc::operation::mode_e" };
// };
//
// template <>
// struct glz::meta<tfc::operation::update_message> {
//   using T = tfc::operation::update_message;
//   static constexpr auto value{ glz::object("new_mode", &T::new_mode, "old_mode", &T::old_mode) };
//   static constexpr auto name{ "tfc::operation::update_message" };
// };
