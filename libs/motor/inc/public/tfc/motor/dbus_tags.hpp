#pragma once

#include <string_view>

#include <fmt/format.h>
#include <mp-units/systems/si/si.h>

#include <tfc/motor/errors.hpp>
#include <tfc/dbus/string_maker.hpp>

namespace tfc::motor::dbus {
namespace detail {
static constexpr std::string_view path_postfix{ "Motors" };
static constexpr std::string_view service{ "Ethercat" };
}  // namespace detail

static constexpr std::string_view service_name{ tfc::dbus::const_dbus_name<detail::service> };
static constexpr std::string_view path{ tfc::dbus::const_dbus_path<detail::path_postfix> };

static inline auto make_interface_name(std::string_view implementation_name, std::uint16_t slave_id) -> std::string {
  return tfc::dbus::make_dbus_name(fmt::format("{}_{}", implementation_name, slave_id));
}

namespace method {
static constexpr std::string_view ping{ "Ping" };
static constexpr std::string_view run{ "Run" };
static constexpr std::string_view run_at_speedratio{ "RunAtSpeedratio" };
static constexpr std::string_view run_at_speedratio_microsecond{ "RunAtSpeedratio_Microsecond" };
static constexpr std::string_view run_microsecond{ "Run_Microsecond" };
static constexpr std::string_view stop{ "Stop" };
static constexpr std::string_view quick_stop{ "QuickStop" };
static constexpr std::string_view needs_homing{ "NeedsHoming" };
static constexpr std::string_view reset{ "Reset" };
// D-Bus does not support method overloading
// todo do metaprogramming deducing postfix from type, when registering methods
static constexpr std::string_view convey_micrometrepersecond_micrometre{ "Convey_MicrometrePerSecond_Micrometre" };
static constexpr std::string_view convey_micrometrepersecond_microsecond{ "Convey_MicrometrePerSecond_Microsecond" };
static constexpr std::string_view convey_micrometre{ "Convey_Micrometre" };
static constexpr std::string_view convey_microsecond{ "Convey_Microsecond" };
static constexpr std::string_view move_speedratio_micrometre{ "Move_Speedratio_Micrometre" };
static constexpr std::string_view move_micrometre{ "Move_Micrometre" };
static constexpr std::string_view move_home{ "Move_Home" };
static constexpr std::string_view notify_after_micrometre{ "NotifyAfter_Micrometre" };
static constexpr std::string_view notify_from_home_micrometre{ "NotifyFromHome_Micrometre" };
}  // namespace method

namespace types {
using micrometre_t = mp_units::quantity<mp_units::si::micro<mp_units::si::metre>, std::int64_t>;
using microsecond_t = mp_units::quantity<mp_units::si::micro<mp_units::si::second>, std::int64_t>;
using speedratio_t = mp_units::quantity<mp_units::percent, double>;
using velocity_t = mp_units::quantity<micrometre_t::reference / mp_units::si::second, std::int64_t>;
}  // namespace types

namespace message {

template <mp_units::Quantity quantity_t>
struct generic {
  errors::err_enum err{ errors::err_enum::unknown };
  quantity_t length{};
  static constexpr auto dbus_reflection{ [](auto&& self) {
    return stx::to_tuple(std::forward<decltype(self)>(self));
  } };
};

using length = generic<types::micrometre_t>;

}


}  // namespace tfc::motor::dbus
