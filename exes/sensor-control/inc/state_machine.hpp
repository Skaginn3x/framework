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
struct stop {};
struct start {};
}  // namespace events

namespace states {
// clang-format off
struct idle { static constexpr std::string_view name{ "idle" }; };
struct awaiting_discharge { static constexpr std::string_view name{ "awaiting_discharge" }; };
struct awaiting_sensor { static constexpr std::string_view name{ "awaiting_sensor" }; };
struct discharging { static constexpr std::string_view name{ "discharging" }; };
struct discharge_delayed { static constexpr std::string_view name{ "discharge_delayed" }; };
struct stopped { static constexpr std::string_view name{ "stopped" }; };
// clang-format on
}

template <typename owner_t>
struct state_machine {
  using self = state_machine;

  auto operator()() {
    using boost::sml::_;
    using boost::sml::event;
    using boost::sml::state;
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
    static constexpr auto save_time_left = [](owner_t& owner) { owner.save_time_left(); };

    // TODO handle stop signals -> stopped or emergency !!!

    // clang-format off
    PRAGMA_CLANG_WARNING_PUSH_OFF(-Wused-but-marked-unused) // Todo fix sml.hpp
    auto table = boost::sml::make_transition_table(
      * state<states::idle> + on_entry<_> / enter_idle
      , state<states::idle> + on_exit<_> / leave_idle
      , state<states::idle> + event<events::sensor_active> = state<states::awaiting_discharge>
      , state<states::idle> + event<events::new_info> = state<states::awaiting_sensor>
      , state<states::awaiting_discharge> + on_entry<_> / enter_awaiting_discharge
      , state<states::awaiting_discharge> + on_exit<_> / leave_awaiting_discharge

      , state<states::awaiting_sensor> + on_entry<_> / enter_awaiting_sensor
      , state<states::awaiting_sensor> + on_exit<_> / leave_awaiting_sensor
      , state<states::awaiting_sensor> + event<events::sensor_active> = state<states::awaiting_discharge>

      , state<states::awaiting_discharge> + event<events::discharge> = state<states::discharging>
      , state<states::discharging> + on_entry<_> / enter_discharging
      , state<states::discharging> + on_exit<_> / leave_discharging

      , state<states::discharging> + event<events::sensor_inactive> [not_using_discharge_delay] = state<states::idle>

      , state<states::discharging> + event<events::sensor_inactive> [using_discharge_delay] = state<states::discharge_delayed>
      , state<states::discharge_delayed> + on_entry<_> / enter_discharge_delayed
      , state<states::discharge_delayed> + on_exit<_> / leave_discharge_delayed

      , state<states::discharge_delayed> + event<events::complete> = state<states::idle>

      , state<states::idle> + event<events::stop> = state<states::stopped>
      , state<states::awaiting_discharge> + event<events::stop> = state<states::stopped>
      , state<states::awaiting_sensor> + event<events::stop> = state<states::stopped>
      , state<states::discharging> + event<events::stop> = state<states::stopped>
      , state<states::discharge_delayed> + event<events::stop> / save_time_left  = state<states::stopped>

      , state<states::stopped> + event<events::start> = state<states::idle>
      , state<states::stopped> + event<events::start> = state<states::awaiting_discharge>
      , state<states::stopped> + event<events::start> = state<states::awaiting_sensor>
      , state<states::stopped> + event<events::start> = state<states::discharging>
      , state<states::stopped> + event<events::start> / save_time_left  = state<states::discharge_delayed>
    );
    PRAGMA_CLANG_WARNING_POP
    // clang-format on
    return table;
  }
};

}  // namespace tfc::sensor::control
