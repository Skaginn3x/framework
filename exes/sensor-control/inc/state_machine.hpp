#pragma once

#include <boost/sml.hpp>

#include <tfc/utils/pragmas.hpp>

namespace tfc::sensor::control {

namespace events {
struct sensor_active {};
struct new_info {};
struct discharge {};
struct complete {};
}  // namespace events

template <typename owner_t>
struct state_machine {
  explicit state_machine(owner_t& owner) : owner_{ owner } {}

  auto operator()() {
    using boost::sml::_;
    using boost::sml::event;
    using boost::sml::on_entry;
    using boost::sml::on_exit;
    using boost::sml::literals::operator""_s;
    using boost::sml::literals::operator""_e;

    // clang-format off
    PRAGMA_CLANG_WARNING_PUSH_OFF(-Wused-but-marked-unused) // Todo fix sml.hpp
    auto table = boost::sml::make_transition_table(
      * "idle"_s + on_entry<_> / [this](){ owner_.enter_idle(); }
      , "idle"_s + on_exit<_> / [this](){ owner_.leave_idle(); }
      , "idle"_s + event<events::sensor_active> = "awaiting_discharge"_s
      , "idle"_s + event<events::new_info> = "awaiting_sensor"_s
      , "awaiting_discharge"_s + on_entry<_> / [this](){ owner_.enter_awaiting_discharge(); }
      , "awaiting_discharge"_s + on_exit<_> / [this](){ owner_.leave_awaiting_discharge(); }

      , "awaiting_sensor"_s + on_entry<_> / [this](){ owner_.enter_awaiting_sensor(); }
      , "awaiting_sensor"_s + on_exit<_> / [this](){ owner_.leave_awaiting_sensor(); }
      , "awaiting_sensor"_s + event<events::sensor_active> = "awaiting_discharge"_s

      , "awaiting_discharge"_s + event<events::discharge> = "discharging"_s
      , "discharging"_s + on_entry<_> / [this](){ owner_.enter_discharging(); }
      , "discharging"_s + on_exit<_> / [this](){ owner_.leave_discharging(); }

      , "discharging"_s + event<events::complete> = "idle"_s
    );
    PRAGMA_CLANG_WARNING_POP
    // clang-format on
    return table;
  }

private:
  owner_t& owner_;
};

}  // namespace tfc::sensor::control
