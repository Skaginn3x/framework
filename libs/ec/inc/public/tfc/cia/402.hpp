#pragma once

#include <boost/sml.hpp>
#include <cstdint>

namespace tfc::ec::cia{

namespace sml = boost::sml;
using sml::literals::operator""_s;

// State machine events
struct reset{ static constexpr uint16_t cmd = 0x0080; };
struct disable_voltage { static constexpr uint16_t cmd = 0x0000; };
struct quick_stop { static constexpr uint16_t cmd = 0x0002; };
struct shutdown { static constexpr uint16_t cmd = 0x0006; };
struct freewheel_stop{};
struct safe_torque_off{};
struct configuration_modification{};
struct enable_operation{};
struct disable_operation{};
struct switch_on{static constexpr uint16_t cmd = 0x0007; };

struct state_machine {
  auto operator()() const {
    return sml::make_transition_table(
          *"start"_s = "not_ready"_s,
          "disabled"_s + sml::event<shutdown> = "ready"_s,
          "ready"_s + sml::event<switch_on> = "on"_s,
          "ready"_s + sml::event<enable_operation> = "enabled"_s,
          "ready"_s + sml::event<disable_voltage> = "disabled"_s,
          "ready"_s + sml::event<quick_stop> = "disabled"_s,
          "ready"_s + sml::event<safe_torque_off> = "disabled"_s,
          "on"_s + sml::event<disable_voltage> = "disabled"_s,
          "on"_s + sml::event<quick_stop> = "disabled"_s,
          "on"_s + sml::event<freewheel_stop> = "disabled"_s,
          "on"_s + sml::event<safe_torque_off> = "disabled"_s,
          "on"_s + sml::event<enable_operation> = "enabled"_s,
          "quick_stop_active"_s + sml::event<quick_stop> = "disabled"_s,
          "quick_stop_active"_s + sml::event<disable_voltage> = "disabled"_s,
          "quick_stop_active"_s + sml::event<freewheel_stop> = "disabled"_s,
          "quick_stop_active"_s + sml::event<shutdown> = "disabled"_s,
          "not_ready"_s = "disabled"_s,
          "Fault"_s + sml::event<reset> = "disabled"_s
        );
  }
};

}  // namespace tfc::ec::cia