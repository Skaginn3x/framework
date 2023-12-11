#pragma once

#include <bit>
#include <cstdint>
#include <string>

#include <tfc/ec/soem_interface.hpp>

namespace tfc::ec::cia_402 {

using ecx::index_t;

struct control_word {
  static constexpr index_t index{ 0x6040, 0x0 };
  bool switch_on : 1 {};
  bool enable_voltage : 1 {};
  bool quick_stop : 1 {};  // FYI, enabled low
  bool enable_operation : 1 {};
  bool operating_mode_specific_0 : 1 {};
  bool operating_mode_specific_1 : 1 {};
  bool operating_mode_specific_2 : 1 {};
  bool fault_reset : 1 {};
  bool halt : 1 {};
  bool operating_mode_specific_3 : 1 {};
  bool reserved_0 : 1 {};
  bool reserved_1 : 1 {};
  bool reserved_2 : 1 {};
  bool reserved_3 : 1 {};
  bool reserved_4 : 1 {};
  bool reserved_5 : 1 {};

  static constexpr auto from_uint(control_word word) noexcept -> control_word { return std::bit_cast<control_word>(word); }

  constexpr explicit operator uint16_t() const noexcept { return std::bit_cast<uint16_t>(*this); }
  auto constexpr operator==(control_word const&) const noexcept -> bool = default;

  // Command factories
  struct commands {
    [[nodiscard]] static auto shutdown() -> control_word {
      return control_word{ .switch_on = false, .enable_voltage = true, .quick_stop = true };
    }
    [[nodiscard]] static auto switch_on() -> control_word {
      return control_word{ .switch_on = true, .enable_voltage = true, .quick_stop = true };
    }
    [[nodiscard]] static auto enable_operation() -> control_word {
      return control_word{ .switch_on = true, .enable_voltage = true, .quick_stop = true, .enable_operation = true };
    }
    [[nodiscard]] static auto disable_operation() -> control_word {
      return control_word{ .switch_on = true, .enable_voltage = true, .quick_stop = true, .enable_operation = false };
    }
    [[nodiscard]] static auto disable_voltage() -> control_word { return control_word{ .enable_voltage = false }; }
    [[nodiscard]] static auto quick_stop() -> control_word { return control_word{ .enable_voltage = true }; }
    [[nodiscard]] static auto fault_reset() -> control_word { return control_word{ .fault_reset = true }; }
  };
};

using commands = control_word::commands;

static_assert(sizeof(control_word) == 2);

enum struct states_e : uint16_t {
  not_ready_to_switch_on = 1,
  switch_on_disabled = 2,
  ready_to_switch_on = 3,
  switched_on = 4,
  operation_enabled = 5,
  quick_stop_active = 6,
  fault_reaction_active = 7,
  fault = 8
};

struct status_word {
  static constexpr index_t index{ 0x6041, 0x0 };
  bool state_ready_to_switch_on : 1 {};
  bool state_switched_on : 1 {};
  bool state_operation_enabled : 1 {};
  bool state_fault : 1 {};
  bool voltage_enabled : 1 {};
  bool state_quick_stop : 1 {};
  bool state_switch_on_disabled : 1 {};
  bool warning : 1 {};
  bool halt_request_active : 1 {};
  bool remote : 1 {};
  bool target_reached : 1 {};
  bool internal_limit_active : 1 {};
  bool application_specific_0 : 1 {};  // 12th bit
  bool application_specific_1 : 1 {};
  bool application_specific_2 : 1 {};
  bool application_specific_3 : 1 {};

  constexpr explicit operator uint16_t() const noexcept { return std::bit_cast<uint16_t>(*this); }

  [[nodiscard]] auto constexpr parse_state() const noexcept -> states_e {
    /*
      Status Word Bit Mapping:
      Bit 0: Ready to switch on.
      Bit 1: Switched on.
      Bit 2: Operation enabled.
      Bit 3: Fault.
      Bit 4: Voltage enabled.
      Bit 5: Quick stop. (FYI, enabled low)
      Bit 6: Switch on disabled.

      Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 | State
      ------------------------------------------------------------
        0   |   x   |   x   |   0   |   0   |   0   |   0   | State 1: Not ready to switch on
        1   |   x   |   x   |   0   |   0   |   0   |   0   | State 2: Switch on disabled
        0   |   1   |   x   |   0   |   0   |   0   |   1   | State 3: Ready to switch on
        0   |   1   |   1   |   0   |   0   |   1   |   1   | State 4: Switched on
        0   |   1   |   1   |   0   |   1   |   1   |   1   | State 5: Operation enabled
        0   |   0   |   1   |   0   |   1   |   1   |   1   | State 6: Quick stop active
        0   |   x   |   x   |   1   |   1   |   1   |   1   | State 7: Fault reaction active
        0   |   x   |   x   |   1   |   0   |   0   |   0   | State 8: Fault
    */

    if (state_fault) {
      if (state_operation_enabled && state_switched_on && state_ready_to_switch_on) {
        return states_e::fault_reaction_active;
      }
      return states_e::fault;
    }
    if (!state_ready_to_switch_on && !state_switched_on && !state_operation_enabled && !state_switch_on_disabled) {
      return states_e::not_ready_to_switch_on;
    }
    if (state_switch_on_disabled) {
      return states_e::switch_on_disabled;
    }
    if (!state_quick_stop) {
      return states_e::quick_stop_active;
    }
    if (state_ready_to_switch_on && state_quick_stop && !state_switched_on) {
      return states_e::ready_to_switch_on;
    }
    if (state_ready_to_switch_on && state_switched_on && voltage_enabled) {
      if (state_operation_enabled) {
        return states_e::operation_enabled;
      }
      return states_e::switched_on;
    }
    return states_e::not_ready_to_switch_on;
  }

  static constexpr auto from_uint(uint16_t word) noexcept -> status_word { return std::bit_cast<status_word>(word); }
};

static_assert(sizeof(status_word) == 2);

[[maybe_unused]] static auto to_string(states_e value) -> std::string {
  using std::string_literals::operator""s;
  switch (value) {
    case states_e::not_ready_to_switch_on:
      return "Not ready to switch on"s;
    case states_e::switch_on_disabled:
      return "Switch on disabled"s;
    case states_e::ready_to_switch_on:
      return "Ready to switch on"s;
    case states_e::switched_on:
      return "Switched on"s;
    case states_e::operation_enabled:
      return "Operation enabled"s;
    case states_e::quick_stop_active:
      return "Quick stop active"s;
    case states_e::fault_reaction_active:
      return "Fault reaction active"s;
    case states_e::fault:
      return "Fault"s;
  }
  return "unknown"s;
}

/**
 * Transition to operational mode
 * @param current_state State parsed from status word determined to be the current status of a drive
 * @param quick_stop if the drive should be placed in quick_stop mode.
 * @return the command to transition to operational mode / stick in quick stop mode.
 */
inline auto transition(states_e current_state, bool run, bool quick_stop, bool freewheel_stop) -> control_word {
  switch (current_state) {
    case states_e::switch_on_disabled:
      return commands::shutdown();
    case states_e::switched_on:
    case states_e::ready_to_switch_on:
      if (run) {
        return commands::enable_operation();  // This is a shortcut marked as 3B in ethercat manual for atv320
      }
      return commands::disable_operation();  // Stay in this state if in ready to switch on else transition to switched on
    case states_e::operation_enabled:
      if (quick_stop) {
        return commands::quick_stop();
      }
      if (freewheel_stop) {
        return commands::disable_voltage();  // Freewheel stop
      }
      if (!run) {
        return commands::shutdown();
      }

      return commands::enable_operation();
    case states_e::fault:
      return commands::fault_reset();

    case states_e::quick_stop_active:
    case states_e::not_ready_to_switch_on:
    case states_e::fault_reaction_active:
      return commands::disable_voltage();
  }
  // Can only occur if someone casts an integer for state_e that is not defined in the enum
  return commands::disable_voltage();
}

}  // namespace tfc::ec::cia_402
