#pragma once

#include <string_view>
#include <variant>

#include <boost/sml.hpp>

#include <tfc/ipc/item.hpp>
#include <tfc/utils/pragmas.hpp>

namespace tfc::sensor::control {

namespace events {
struct sensor_active {
  static constexpr std::string_view name{ "sensor_active" };
};
struct sensor_inactive {
  static constexpr std::string_view name{ "sensor_inactive" };
};
struct await_sensor_timeout {
  static constexpr std::string_view name{ "await_sensor_timeout" };
};
struct new_info {
  static constexpr std::string_view name{ "new_info" };
};
struct discharge {
  static constexpr std::string_view name{ "discharge" };
};
struct complete {
  static constexpr std::string_view name{ "complete" };
};
struct stop {
  static constexpr std::string_view name{ "stop" };
};
struct start {
  static constexpr std::string_view name{ "start" };
};
}  // namespace events

namespace states {
// clang-format off
struct idle { static constexpr std::string_view name{ "idle" }; };
struct awaiting_discharge { static constexpr std::string_view name{ "awaiting_discharge" }; };
struct awaiting_sensor { static constexpr std::string_view name{ "awaiting_sensor" }; };
struct uncontrolled_discharge { static constexpr std::string_view name{ "uncontrolled_discharge" }; };
struct discharging { static constexpr std::string_view name{ "discharging" }; };
struct discharge_delayed { static constexpr std::string_view name{ "discharge_delayed" }; };
struct discharging_allow_input{ static constexpr std::string_view name{ "discharging_allow_input" }; };
// clang-format on
}  // namespace states

namespace guards {
template <typename owner_t>
struct is_controlled_discharge {
  static constexpr std::string_view name{ "is_controlled_discharge" };
  auto operator()(owner_t const& owner) const noexcept -> bool { return owner.config_run_on_discharge(); }
};
template <typename owner_t>
struct is_uncontrolled_discharge {
  static constexpr std::string_view name{ "is_uncontrolled_discharge" };
  auto operator()(owner_t const& owner) const noexcept -> bool { return is_controlled_discharge<owner_t>{}(owner); }
};
template <typename owner_t>
struct using_discharge_delay {
  static constexpr std::string_view name{ "using_discharge_delay" };
  auto operator()(owner_t const& owner) const noexcept -> bool { return owner.using_discharge_delay(); }
};
template <typename owner_t>
struct not_using_discharge_delay {
  static constexpr std::string_view name{ "not_using_discharge_delay" };
  auto operator()(owner_t const& owner) const noexcept -> bool { return !owner.using_discharge_delay(); }
};
}  // namespace guards

namespace actions {
template <typename owner_t>
struct enter_idle {
  static constexpr std::string_view name{ "enter_idle" };
  auto operator()(owner_t& owner) const noexcept { owner.enter_idle(); }
};
template <typename owner_t>
struct leave_idle {
  static constexpr std::string_view name{ "leave_idle" };
  auto operator()(owner_t& owner) const noexcept { owner.leave_idle(); }
};
template <typename owner_t>
struct enter_awaiting_discharge {
  static constexpr std::string_view name{ "enter_awaiting_discharge" };
  auto operator()(owner_t& owner) const noexcept { owner.enter_awaiting_discharge(); }
};
template <typename owner_t>
struct leave_awaiting_discharge {
  static constexpr std::string_view name{ "leave_awaiting_discharge" };
  auto operator()(owner_t& owner) const noexcept { owner.leave_awaiting_discharge(); }
};
template <typename owner_t>
struct enter_awaiting_sensor {
  static constexpr std::string_view name{ "enter_awaiting_sensor" };
  auto operator()(owner_t& owner) const noexcept { owner.enter_awaiting_sensor(); }
};
template <typename owner_t>
struct leave_awaiting_sensor {
  static constexpr std::string_view name{ "leave_awaiting_sensor" };
  auto operator()(owner_t& owner) const noexcept { owner.leave_awaiting_sensor(); }
};
template <typename owner_t>
struct enter_discharging {
  static constexpr std::string_view name{ "enter_discharging" };
  auto operator()(owner_t& owner) const noexcept { owner.enter_discharging(); }
};
template <typename owner_t>
struct leave_discharging {
  static constexpr std::string_view name{ "leave_discharging" };
  auto operator()(owner_t& owner) const noexcept { owner.leave_discharging(); }
};
template <typename owner_t>
struct enter_uncontrolled_discharge {
  static constexpr std::string_view name{ "enter_uncontrolled_discharge" };
  auto operator()(owner_t& owner) const noexcept { owner.enter_uncontrolled_discharge(); }
};
template <typename owner_t>
struct leave_uncontrolled_discharge {
  static constexpr std::string_view name{ "leave_uncontrolled_discharge" };
  auto operator()(owner_t& owner) const noexcept { owner.leave_uncontrolled_discharge(); }
};
template <typename owner_t>
struct enter_discharge_delayed {
  static constexpr std::string_view name{ "enter_discharge_delayed" };
  auto operator()(owner_t& owner) const noexcept { owner.enter_discharge_delayed(); }
};
template <typename owner_t>
struct leave_discharge_delayed {
  static constexpr std::string_view name{ "leave_discharge_delayed" };
  auto operator()(owner_t& owner) const noexcept { owner.leave_discharge_delayed(); }
};
template <typename owner_t>
struct enter_discharging_allow_input {
  static constexpr std::string_view name{ "enter_discharging_allow_input" };
  auto operator()(owner_t& owner) const noexcept { owner.enter_discharging_allow_input(); }
};
template <typename owner_t>
struct leave_discharging_allow_input {
  static constexpr std::string_view name{ "leave_discharging_allow_input" };
  auto operator()(owner_t& owner) const noexcept { owner.leave_discharging_allow_input(); }
};
}  // namespace actions

template <typename owner_t>
struct state_machine {
  static constexpr std::string_view name{ "tfc::sensor::control::state_machine" };

  auto operator()() {
    using boost::sml::_;
    using boost::sml::event;
    using boost::sml::on_entry;
    using boost::sml::on_exit;
    using boost::sml::state;
    using boost::sml::literals::operator""_s;
    using boost::sml::literals::operator""_e;
    using boost::sml::H;

    // clang-format off
    PRAGMA_CLANG_WARNING_PUSH_OFF(-Wused-but-marked-unused) // Todo fix sml.hpp
    auto table = boost::sml::make_transition_table(
      state<states::idle>(H) + on_entry<_> / actions::enter_idle<owner_t>{}
      , state<states::idle> + on_exit<_> / actions::leave_idle<owner_t>{}
      , state<states::idle> + event<events::sensor_active> = state<states::awaiting_discharge>
      , state<states::idle> + event<events::new_info> = state<states::awaiting_sensor>

      , state<states::awaiting_discharge> + on_entry<_> / actions::enter_awaiting_discharge<owner_t>{}
      , state<states::awaiting_discharge> + on_exit<_> / actions::leave_awaiting_discharge<owner_t>{}
      , state<states::awaiting_discharge> + event<events::discharge> [guards::is_controlled_discharge<owner_t>{}] = state<states::discharging>
      , state<states::awaiting_discharge> + event<events::discharge> [guards::is_uncontrolled_discharge<owner_t>{}] = state<states::uncontrolled_discharge>
      , state<states::awaiting_discharge> + event<events::sensor_inactive> = state<states::idle> // todo test

      , state<states::uncontrolled_discharge> + on_entry<_> / actions::enter_uncontrolled_discharge<owner_t>{}
      , state<states::uncontrolled_discharge> + on_exit<_> / actions::leave_uncontrolled_discharge<owner_t>{}

      , state<states::awaiting_sensor> + on_entry<_> / actions::enter_awaiting_sensor<owner_t>{}
      , state<states::awaiting_sensor> + on_exit<_> / actions::leave_awaiting_sensor<owner_t>{}
      , state<states::awaiting_sensor> + event<events::sensor_active> = state<states::awaiting_discharge>
      , state<states::awaiting_sensor> + event<events::await_sensor_timeout> = state<states::idle>

      , state<states::discharging> + on_entry<_> / actions::enter_discharging<owner_t>{}
      , state<states::discharging> + on_exit<_> / actions::leave_discharging<owner_t>{}

      , state<states::discharging> + event<events::sensor_inactive> [guards::not_using_discharge_delay<owner_t>{}] = state<states::idle>
      , state<states::discharging> + event<events::sensor_inactive> [guards::using_discharge_delay<owner_t>{}] = state<states::discharge_delayed>
      , state<states::discharging> + event<events::new_info> = state<states::discharging_allow_input>

      , state<states::uncontrolled_discharge> + event<events::sensor_inactive> [guards::not_using_discharge_delay<owner_t>{}] = state<states::idle>
      , state<states::uncontrolled_discharge> + event<events::sensor_inactive> [guards::using_discharge_delay<owner_t>{}] = state<states::discharge_delayed>

      , state<states::discharge_delayed> + on_entry<_> / actions::enter_discharge_delayed<owner_t>{}
      , state<states::discharge_delayed> + on_exit<_> / actions::leave_discharge_delayed<owner_t>{}

      , state<states::discharge_delayed> + event<events::complete> = state<states::idle>
      , state<states::discharge_delayed> + event<events::new_info> = state<states::discharging_allow_input>

      , state<states::discharging_allow_input> + on_entry<_> / actions::enter_discharging_allow_input<owner_t>{}
      , state<states::discharging_allow_input> + on_exit<_> / actions::leave_discharging_allow_input<owner_t>{}
      , state<states::discharging_allow_input> + event<events::sensor_inactive> = state<states::awaiting_sensor>
    );
    PRAGMA_CLANG_WARNING_POP
    // clang-format on
    return table;
  }
};

template <typename owner_t>
struct state_machine_operation_mode {
  auto operator()() {
    using boost::sml::literals::operator""_s;
    using boost::sml::_;
    using boost::sml::event;
    using boost::sml::on_entry;
    using boost::sml::on_exit;
    using boost::sml::state;

    static constexpr auto enter_stopped = [](owner_t& owner) { owner.enter_stopped(); };
    static constexpr auto leave_stopped = [](owner_t& owner) { owner.leave_stopped(); };

    static constexpr auto enter_running = [](owner_t& owner) { owner.enter_running(); };
    static constexpr auto leave_running = [](owner_t& owner) { owner.leave_running(); };

    // clang-format off
    PRAGMA_CLANG_WARNING_PUSH_OFF(-Wused-but-marked-unused) // Todo fix sml.hpp
    // clang-format on
    return boost::sml::make_transition_table(
        // clang-format off
        * "stopped"_s + on_entry<_> / enter_stopped
        , "stopped"_s + on_exit<_> / leave_stopped
        , "stopped"_s + event<events::start> = state<state_machine<owner_t>>

        , state<state_machine<owner_t>> + on_entry<_> / enter_running
        , state<state_machine<owner_t>> + on_exit<_> / leave_running
        , state<state_machine<owner_t>> + event<events::stop> = "stopped"_s
        // clang-format on
    );
    PRAGMA_CLANG_WARNING_POP
  }
};

}  // namespace tfc::sensor::control
