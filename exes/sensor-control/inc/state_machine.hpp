#pragma once

#include <boost/sml.hpp>

#include <tfc/utils/pragmas.hpp>

namespace tfc::sensor::control {

namespace events {
struct sensor_active {};
struct sensor_inactive {};
struct new_info {};
struct discharge {};
struct complete {};
}  // namespace events

template <typename owner_t>
struct state_machine {
  using self = state_machine;

  auto operator()() {
    using boost::sml::_;
    using boost::sml::event;
    using boost::sml::on_entry;
    using boost::sml::on_exit;
    using boost::sml::literals::operator""_s;
    using boost::sml::literals::operator""_e;

    static constexpr auto enter_idle = [](owner_t& owner) { owner.enter_idle(); };
    static constexpr auto leave_idle = [](owner_t& owner) { owner.leave_idle(); };
    static constexpr auto enter_awaiting_discharge = [](owner_t& owner) { owner.enter_awaiting_discharge(); };
    static constexpr auto leave_awaiting_discharge = [](owner_t& owner) { owner.leave_awaiting_discharge(); };
    static constexpr auto enter_awaiting_sensor = [](owner_t& owner) { owner.enter_awaiting_sensor(); };
    static constexpr auto leave_awaiting_sensor = [](owner_t& owner) { owner.leave_awaiting_sensor(); };
    static constexpr auto enter_discharging = [](owner_t& owner) { owner.enter_discharging(); };
    static constexpr auto leave_discharging = [](owner_t& owner) { owner.leave_discharging(); };
    static constexpr auto enter_discharge_delayed = [](owner_t& owner) { owner.enter_discharge_delayed(); };
    static constexpr auto leave_discharge_delayed = [](owner_t& owner) { owner.leave_discharge_delayed(); };

    static constexpr auto using_discharge_delay = [](owner_t& owner) { return owner.using_discharge_delay(); };
    static constexpr auto not_using_discharge_delay = [](owner_t& owner) { return !owner.using_discharge_delay(); };

    // clang-format off
    PRAGMA_CLANG_WARNING_PUSH_OFF(-Wused-but-marked-unused) // Todo fix sml.hpp
    auto table = boost::sml::make_transition_table(
      * "idle"_s + on_entry<_> / enter_idle
      , "idle"_s + on_exit<_> / leave_idle
      , "idle"_s + event<events::sensor_active> = "awaiting_discharge"_s
      , "idle"_s + event<events::new_info> = "awaiting_sensor"_s
      , "awaiting_discharge"_s + on_entry<_> / enter_awaiting_discharge
      , "awaiting_discharge"_s + on_exit<_> / leave_awaiting_discharge

      , "awaiting_sensor"_s + on_entry<_> / enter_awaiting_sensor
      , "awaiting_sensor"_s + on_exit<_> / leave_awaiting_sensor
      , "awaiting_sensor"_s + event<events::sensor_active> = "awaiting_discharge"_s

      , "awaiting_discharge"_s + event<events::discharge> = "discharging"_s
      , "discharging"_s + on_entry<_> / enter_discharging
      , "discharging"_s + on_exit<_> / leave_discharging

      , "discharging"_s + event<events::sensor_inactive> [not_using_discharge_delay] = "idle"_s

      , "discharging"_s + event<events::sensor_inactive> [using_discharge_delay] = "discharge_delayed"_s
      , "discharge_delayed"_s + on_entry<_> / enter_discharge_delayed
      , "discharge_delayed"_s + on_exit<_> / leave_discharge_delayed

      , "discharge_delayed"_s + event<events::complete> = "idle"_s
    );
    PRAGMA_CLANG_WARNING_POP
    // clang-format on
    return table;
  }
};

}  // namespace tfc::sensor::control
