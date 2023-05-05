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

namespace concepts {

template <typename struct_t>
concept dbus_reflectable = requires {
  struct_t::dbus_reflection;
  requires std::invocable<decltype(struct_t::dbus_reflection), struct_t&&>;
};

template <typename enum_t>
concept enum_c = requires { std::is_enum_v<enum_t>; };

}  // namespace concepts

namespace sdbusplus::message::types::details {

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

template <concepts::enum_c enum_t>
struct convert_from_string<enum_t> {
  static auto op(const std::string& mode_str) noexcept -> std::optional<enum_t> {
    return magic_enum::enum_cast<enum_t>(mode_str);
  }
};

template <concepts::enum_c enum_t>
struct convert_to_string<enum_t> {
  static auto op(enum_t mode) -> std::string { return std::string{ magic_enum::enum_name(mode) }; }
};

template <typename struct_t, typename enable_t>
struct append_single;

template <concepts::dbus_reflectable struct_t>
struct append_single<struct_t, void> {
  static void op(auto* interface, auto* sd_bus_msg, auto&& item) {
    append_single<decltype(struct_t::dbus_reflection(struct_t{})), void>::op(
        interface, sd_bus_msg, struct_t::dbus_reflection(std::forward<decltype(item)>(item)));
  }
};

template <typename struct_t, typename enable_t>
struct read_single;

template <concepts::dbus_reflectable struct_t>
struct read_single<struct_t, void> {
  static void op(auto* interface, auto* sd_bus_msg, auto& return_value) {
    using return_value_tuple_t = decltype(struct_t::dbus_reflection(struct_t{}));
    return_value_tuple_t return_value_tuple;
    read_single<return_value_tuple_t, void>::op(interface, sd_bus_msg, return_value_tuple);
    return_value = std::make_from_tuple<struct_t>(return_value_tuple);
  }
};

}  // namespace sdbusplus::message::details
