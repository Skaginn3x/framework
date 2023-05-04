
#include <cstdint>
#include <magic_enum.hpp>
#include <string_view>
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
static constexpr std::string_view path{ tfc::dbus::const_dbus_name<service_name> };
namespace signal {
static constexpr std::string_view update{ "update" };
}  // namespace signal
}  // namespace dbus

struct update_message {
  mode_e new_mode{ mode_e::unknown };
  mode_e old_mode{ mode_e::unknown };

  struct refl_dbus {
    using T = up
    static constexpr auto value{ std::tuple() }
  };

};

}  // namespace tfc::operation

namespace sdbusplus::message::types::details {

template <>
struct type_id<tfc::operation::update_message> {
  static constexpr auto value = std::tuple_cat(tuple_type_id_v<SD_BUS_TYPE_STRUCT_BEGIN>,
                                               type_id_v<tfc::operation::mode_e>,
                                               type_id_v<tfc::operation::mode_e>,
                                               tuple_type_id_v<SD_BUS_TYPE_STRUCT_END>);
};

}  // namespace sdbusplus::message::types::details

//template <>
//struct glz::meta<tfc::operation::mode_e> {
//  using enum tfc::operation::mode_e;
//  // clang-format off
//  static constexpr auto value{ glz::enumerate(
//      "unknown", unknown,
//      "stopped", stopped,
//      "running", running,
//      "specialized_running_1", specialized_running_1,
//      "specialized_running_2", specialized_running_2,
//      "specialized_running_3", specialized_running_3,
//      "fault", fault,
//      "cleaning", cleaning,
//      "emergency", emergency,
//      "maintenance", maintenance
//  ) };
//  // clang-format on
//  static constexpr std::string_view name{ "tfc::operation::mode_e" };
//};
//
//template <>
//struct glz::meta<tfc::operation::update_message> {
//  using T = tfc::operation::update_message;
//  static constexpr auto value{ glz::object("new_mode", &T::new_mode, "old_mode", &T::old_mode) };
//  static constexpr auto name{ "tfc::operation::update_message" };
//};
