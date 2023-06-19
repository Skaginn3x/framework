#pragma once

#include "tfc/ec/devices/util.hpp"

namespace tfc::ec::devices::schneider::lxm32m::settings {
using ec::util::setting;

enum struct operation_mode_e : int8_t {  // datasheet says that canopen is only int8_t, but get an error if set as int16_t
  manual_or_autotuning = -6,
  motion_sequence = -3,
  electronic_gear = -2,
  jog = -1,
  reserved = 0,
  profile_position = 1,
  profile_velocity = 3,
  profile_torque = 4,
  homing = 6,
  interpolated_position = 7,
  cyclic_synchronous_position = 8,
  cyclic_synchronous_velocity = 9,
  cyclic_synchronous_torque = 10,
};
using operation_mode =
    setting<ecx::index_t{ 0x6060, 0x0 }, "DCOMopmode", "Mode of operation", operation_mode_e, operation_mode_e::reserved>;

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



}
