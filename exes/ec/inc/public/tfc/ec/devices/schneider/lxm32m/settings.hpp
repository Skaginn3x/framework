#pragma once

#include <utility>

#include "tfc/cia/402.hpp"
#include "tfc/ec/devices/util.hpp"

namespace tfc::ec::devices::schneider::lxm32m::settings {
using ec::util::setting;

// datasheet says that canopen is only int8_t, but get an error if set as int16_t
enum struct operation_mode_e : int8_t {
  // vendor specific operation modes are <= 0
  manual_or_autotuning = -6,
  motion_sequence = -3,
  electronic_gear = -2,
  jog = -1,
  reserved = 0,
  profile_position = std::to_underlying(cia_402::operation_mode_e::profile_position),
  profile_velocity = std::to_underlying(cia_402::operation_mode_e::profile_velocity),
  profile_torque = std::to_underlying(cia_402::operation_mode_e::profile_torque),
  homing = std::to_underlying(cia_402::operation_mode_e::homing),
  interpolated_position = std::to_underlying(cia_402::operation_mode_e::interpolated_position),
  cyclic_synchronous_position = std::to_underlying(cia_402::operation_mode_e::cyclic_synchronous_position),
  cyclic_synchronous_velocity = std::to_underlying(cia_402::operation_mode_e::cyclic_synchronous_velocity),
  cyclic_synchronous_torque = std::to_underlying(cia_402::operation_mode_e::cyclic_synchronous_torque),
};

using operation_mode =
    setting<ecx::index_t{ 0x6060, 0x0 }, "DCOMopmode", "Mode of operation", operation_mode_e, operation_mode_e::reserved>;
using operation_mode_read =
    setting<ecx::index_t{ 0x6061, 0x0 }, "DCOMopmode", "Mode of operation", operation_mode_e, operation_mode_e::reserved>;
// Todo fetch supported drive modes with 0x6502

// CompParSyncMot
// Todo is CompParSyncMot 16 bit unsigned?
enum struct compatibility_for_synchronous_motor_e : uint16_t {
  off = 0,
  on = 1,
};
using compatibility_for_synchronous_motor = setting<ecx::index_t{ 0x3006, 0x3D },
                                                    "CompParSyncMot",
                                                    "Compatibility setting for the Synchronous operating modes",
                                                    compatibility_for_synchronous_motor_e,
                                                    compatibility_for_synchronous_motor_e::off>;
enum struct modulo_enable_e : uint16_t {
  off = 0,
  on = 1,
};
using modulo_enable = setting<ecx::index_t{ 0x3006, 0x38 },
                              "MOD_Enable",
                              "Activation of Modulo function",
                              modulo_enable_e,
                              modulo_enable_e::off>;
enum struct quick_stop_option_e : int16_t {
  torque_ramp_then_fault = -2,        // Use torque ramp and transit to operating state 9 Fault
  deceleration_ramp_then_fault = -1,  // Use deceleration ramp and transit to operating state 9 Fault
  deceleration_ramp = 6,              // Use deceleration ramp and remain in operating state 7 Quick Stop
  torque_ramp = 7,                    // Use torque ramp and remain in operating state 7 Quick Stop
};
using quick_stop_option = setting<ecx::index_t{ 0x3006, 0x18 },
                                  "LIM_QStopReact",
                                  "Type of deceleration for Quick Stop",
                                  quick_stop_option_e,
                                  quick_stop_option_e::deceleration_ramp>;
enum struct response_to_active_limit_e : uint16_t {
  error = 0,     // Active limit switch triggers an error.
  no_error = 1,  // : Active limit switch does not trigger an error.
};
using response_to_active_limit = setting<ecx::index_t{ 0x3006, 0x6 },
                                         "IOsigRespOfPS",
                                         "Response to active limit switch during enabling of power stage",
                                         response_to_active_limit_e,
                                         response_to_active_limit_e::error>;
using position_scaling_denom =
    setting<ecx::index_t{ 0x3006, 0x7 }, "ScalePOSdenom", "Position scaling: Denominator", int32_t, int32_t{ 16384 }>;
using position_scaling_num = setting<ecx::index_t{ 0x3006, 0x8 },
                                     "ScalePOSnum",
                                     "Position scaling: Numerator. Specification of the scaling factor: Motor revolutions",
                                     int32_t,
                                     int32_t{ 1 }>;
// todo does mp units declare a type with fixed precision (digits after the decimal point)
using velocity_feed_forward_control_1 = setting<ecx::index_t{ 0x3012, 0x6 },
                                                "CTRL1_KFPp",
                                                "Velocity feed-forward control 1, value of 1234 means 123.4%",
                                                uint16_t,
                                                0>;
using velocity_feed_forward_control_2 = setting<ecx::index_t{ 0x3013, 0x6 },
                                                "CTRL2_KFPp",
                                                "Velocity feed-forward control 2, value of 1234 means 123.4%",
                                                uint16_t,
                                                0>;
// Todo is ECATinpshifttime 32 bit unsigned? and do we know the default value
using input_shift_time = setting<ecx::index_t{ 0x1C33, 0x3 }, "ECATinpshifttime", "Input shift time", uint32_t, 250000>;

// TODO MP UNITS EVERYWHERE APPLICABLE
using target_velocity = setting<ecx::index_t{ 0x60FF, 0x0 }, "PVv_target", "Target motor velocity", int32_t, 0>;
// todo is this correct:
// https://product-help.schneider-electric.com/Machine%20Expert/V1.1/en/Lx32sDr/Lx32sDr/Functions_for_Operation/Functions_for_Operation-3.htm
// RPM/s
using acceleration_ramp =
    setting<ecx::index_t{ 0x6083, 0x0 },
            "RAMP_v_acc",
            "Acceleration of the motion profile for velocity. Writing the value 0 has no effect on the parameter.",
            uint32_t,
            600>;
// RPM/s
using deceleration_ramp = setting<ecx::index_t{ 0x6084, 0x0 },
                                  "RAMP_v_dec",
                                  "Deceleration of the motion profile for velocity. The minimum value 120 for operating "
                                  "modes: Jog, Homing. Writing the value 0 has no effect on the parameter.",
                                  uint32_t,
                                  600>;

namespace electric_current = units::aliases::isq::si::electric_current;
using actual_motor_current = setting<ecx::index_t{ 0x301E, 0x01 },
                                     "_Iq_act_rms",
                                     "Actual motor current (q component, generating torque)\n"
                                     "In increments of 0.01 Arms.",
                                     electric_current::cA<int16_t>,
                                     0>;
using reference_motor_current = setting<ecx::index_t{ 0x301E, 0x10 },
                                        "_Iq_ref_rms",
                                        "Reference motor current (q component, generating torque)\n"
                                        "In increments of 0.01 Arms.",
                                        electric_current::cA<int16_t>,
                                        0>;

// Warning that requires a reset
struct latched_warning {
  static constexpr ecx::index_t index{ 0x301C, 0x0C };
  bool general : 1 {};
  bool reserved_0 : 1 {};
  bool out_of_range : 1 {};
  bool reserved_1 : 1 {};
  bool active_operating_mode : 1 {};
  bool commissioning_interface_rs485 : 1{};
  bool integrated_fieldbus : 1 {};
  bool reserved_2 : 1 {};
  bool following_error : 1 {};
  bool reserved_3 : 1 {};
  bool inputs_STO_A_and_or_STO_B : 1 {};
  bool reserved_4 : 1 {};
  bool reserved_5 : 1 {};
  bool low_voltage_DC_bus_or_mains_phase_missing : 1 {};
  bool reserved_6 : 1 {};
  bool reserved_7 : 1 {};
  bool integrated_encoder_interface : 1 {};
  bool temperature_of_motor_high : 1 {};
  bool temperature_of_power_stage_high : 1 {};
  bool reserved_8 : 1 {};
  bool memory_card : 1 {};
  bool fieldbus_module : 1 {};
  bool encoder_module : 1 {};
  bool safety_module_eSM_or_module_IOM1 : 1 {};
  bool reserved_9 : 1 {};
  bool reserved_10 : 1 {};
  bool reserved_11 : 1 {};
  bool reserved_12 : 1 {};
  bool transistor_for_braking_resistor_overload : 1 {};
  bool braking_resistor_overload : 1 {};
  bool power_stage_overload : 1 {};
  bool motor_overload : 1 {};
};

static_assert(sizeof(latched_warning) == 4);


}  // namespace tfc::ec::devices::schneider::lxm32m::settings
