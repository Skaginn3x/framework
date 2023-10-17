#pragma once

#include <string_view>
#include <variant>

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
}  // namespace states

template <typename owner_t>
struct state_machine {
  state_machine() = default;
//  state_machine(state_machine const&) = delete;
//  state_machine(state_machine&&) = delete;
//  auto operator=(state_machine const&) -> state_machine& = delete;
//  auto operator=(state_machine&&) -> state_machine& = delete;

  auto last_state_idle() -> bool {
    return std::holds_alternative<states::idle>(last_state_);
  }
  auto last_state_awaiting_discharge() -> bool {
    return std::holds_alternative<states::awaiting_discharge>(last_state_);
  }
  auto last_state_awaiting_sensor() -> bool { return std::holds_alternative<states::awaiting_sensor>(last_state_); }
  auto last_state_discharging() -> bool { return std::holds_alternative<states::discharging>(last_state_); }
  auto last_state_discharge_delayed() -> bool { return std::holds_alternative<states::discharge_delayed>(last_state_); }

  auto operator()() {
    using boost::sml::_;
    using boost::sml::event;
    using boost::sml::on_entry;
    using boost::sml::on_exit;
    using boost::sml::state;
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

      , state<states::idle> + event<events::stop> / [this](){
          last_state_ = states::idle{};
      } = state<states::stopped>
      , state<states::awaiting_discharge> + event<events::stop> / [this](){
          last_state_ = states::awaiting_discharge{};
      } = state<states::stopped>
      , state<states::awaiting_sensor> + event<events::stop> / [this](){
          last_state_ = states::awaiting_sensor{};
      } = state<states::stopped>
      , state<states::discharging> + event<events::stop> / [this](){
          last_state_ = states::discharging{};
      } = state<states::stopped>
      , state<states::discharge_delayed> + event<events::stop> / [this](owner_t& owner){
          last_state_ = states::discharge_delayed{};
          owner.save_time_left();
      } = state<states::stopped>

//      , state<states::stopped> + event<events::start> [&state_machine::last_state_idle] = state<states::idle>
      , state<states::stopped> + event<events::start> [&state_machine::last_state_awaiting_discharge] = state<states::awaiting_discharge>
//      , state<states::stopped> + event<events::start> [&state_machine::last_state_awaiting_sensor] = state<states::awaiting_sensor>
//      , state<states::stopped> + event<events::start> [&state_machine::last_state_discharging] = state<states::discharging>
//      , state<states::stopped> + event<events::start> [&state_machine::last_state_discharge_delayed] = state<states::discharge_delayed>
    );
    PRAGMA_CLANG_WARNING_POP
    // clang-format on
    return table;
  }

  std::variant<states::idle,
               states::awaiting_discharge,
               states::awaiting_sensor,
               states::discharging,
               states::discharge_delayed>
      last_state_{ states::idle{} };
};

}  // namespace tfc::sensor::control
