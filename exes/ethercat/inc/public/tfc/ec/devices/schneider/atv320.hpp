#pragma once

#include <fmt/format.h>
#include <mp-units/math.h>
#include <mp-units/systems/isq/isq.h>
#include <mp-units/systems/si/si.h>
#include <bitset>
#include <memory>
#include <optional>

#include <tfc/cia/402.hpp>
#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/ec/devices/base.hpp>
#include <tfc/ec/devices/util.hpp>
#include <tfc/ec/soem_interface.hpp>
#include <tfc/ipc.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace tfc::ec::devices::schneider {
using tfc::ec::util::setting;
enum struct atv320_aiol_e : uint16_t {
  positive_only = 0,          // 0 - 100%
  positive_and_negative = 1,  // +/- 100%
};

using analog_input_3_range =
    setting<ecx::index_t{ 0x200E, 0x55 }, "AI3L", "Analog input 3 range", atv320_aiol_e, atv320_aiol_e::positive_only>;

enum struct atv320_aiot_e : uint16_t {
  voltage = 1,
  current = 2,
  voltage_bipolar = 5,
  ptc_management = 7,
  kty = 8,
  pt1000 = 9,
  pt100 = 10,
  water_prob = 11,
  pt1000_3 = 12,
  pt100_3 = 13,
  pt1000_3_wires = 14,
  pt100_3_wires = 15,
  pt1000_3_wires_3 = 16,
  pt100_3_wires_3 = 17,
};

using configuration_of_AI1 =
    setting<ecx::index_t{ 0x2010, 0x02 }, "AI1", "Configuration of Analog input 1", atv320_aiot_e, atv320_aiot_e::current>;
using configuration_of_AI3 =
    setting<ecx::index_t{ 0x200E, 0x05 }, "AI3", "Configuration of Analog input 3", atv320_aiot_e, atv320_aiot_e::current>;
enum struct atv320_psa_e : uint16_t {
  not_configured = 0,
  analog_input_1 = 1,
  analog_input_2 = 2,
  analog_input_3 = 3,
  analog_input_4 = 4,
  analog_input_5 = 5,
  motor_current = 129,
  motor_frequency = 130,
  ramp_out = 131,
  motor_torque = 132,
  signed_torque = 133,
  signed_ramp = 134,
  pid_reference = 135,
  pid_feedback = 136,
  pid_error = 137,
  pid_output = 138,
  motor_power = 139,
  motor_thermal_state = 140,
  drive_thermal_state = 141,
  reference_frequency_via_di = 160,
  reference_frequency_via_remote_terminal = 163,
  reference_frequency_via_modbus = 164,
  reference_frequency_via_canopen = 167,
  reference_frequency_via_com_module = 169,
  embedded_ethernet = 171,
  signed_output_frequency = 173,
  motor_voltage = 180,
  ai_virtual_1 = 183,
  ai_virtual_2 = 185,
  ai_virtual_3 = 197,
  di5_pulse_input_assignment = 186,
  di6_pulse_input_assignment = 187,
  estimated_pump_flow = 340,
  inlet_pressure = 341,
  outlet_pressure = 342,
  installation_flow = 343,
  estimated_pump_system_flow = 346,
  estimated_pump_head = 347,
  estimated_pump_delta_pressure = 348,
  estimated_pump_system_delta_pressure = 349,
  none = 510,
};
using configuration_reference_frequency_1_FR1 = setting<ecx::index_t{ 0x2036, 0xE },
                                                        "FR1",
                                                        "Configuration reference frequency 1",
                                                        atv320_psa_e,
                                                        atv320_psa_e::reference_frequency_via_com_module>;

using assignment_AQ1 =
    setting<ecx::index_t{ 0x2014, 0x16 }, "AO1", "AQ1 assignment", atv320_psa_e, atv320_psa_e::not_configured>;

enum struct atv320_psl_e : uint16_t {
  not_assigned = 0,
  drive_in_operating_state_fault = 1,
  drive_running = 2,
  ouput_contactor_control = 3,
  motor_frequency_high_threshold_reached = 4,
  high_speed_reached = 5,
  current_threshold_reached = 6,
  reference_frequency_reached = 7,
  motor_thermal_threshold_reached = 8,
  pid_error_warning = 10,
  pid_feedback_warning = 11,
  ai2_4_20_loss_warning = 12,
  motor_frequency_high_threshold_2_reached = 13,
  drive_thermal_threshold_reached = 14,
  reference_frequency_high_threshold_reached = 16,
  reference_frequency_low_threshold_reached = 17,
  motor_frequency_low_threshold_reached = 18,
  motor_frequency_low_threshold_2_reached = 19,
  low_current_threshold_reached = 20,
  process_underload_warning = 21,
  process_overload_warning = 22,
  pid_high_feedback_warning = 23,
  pid_low_feedback_warning = 24,
  regulation_warning = 25,
  forced_run = 26,
  high_torque_warning = 28,
  low_torque_warning = 29,
  run_forward = 30,
  run_reverse = 31,
  ramp_switching = 34,
  hmi_command = 42,
  negative_torque = 47,
  configuration_0_active = 48,
  parameter_set_1_active = 52,
  parameter_set_2_active = 53,
  parameter_set_3_active = 54,
  parameter_set_4_active = 55,
  dc_bus_charged = 64,
  power_removal_state = 66,
  mains_contactor_control = 73,
  i_present = 77,
  warning_group_1 = 80,
  warning_group_2 = 81,
  warning_group_3 = 82,
  external_error_warning = 87,
  undervoltage_warning = 88,
  preventive_undervoltage_active = 89,
  drive_thermal_state_warning = 91,
  afe_mains_undervoltage = 92,
  reference_frequency_channel_1 = 96,
  reference_frequency_channel_2 = 97,
  command_channel_1 = 98,
  command_channel_2 = 99,
  command_ch_1b = 100,
  igbt_thermal_warning = 104,
  ai3_4_20_loss_warning = 107,
  ai4_4_20_loss_warning = 108,
  flow_limit_active = 110,
  graphic_display_terminal_function_key_1 = 116,
  graphic_display_terminal_function_key_2 = 117,
  graphic_display_terminal_function_key_3 = 118,
  graphic_display_terminal_function_key_4 = 119,
  ai1_4_20_loss_warning = 123,
  ready = 127,
  yes = 128,
  digital_input_1 = 129,
  digital_input_2 = 130,
  digital_input_3 = 131,
  digital_input_4 = 132,
  digital_input_5 = 133,
  digital_input_6 = 134,
  digital_input_11 = 139,
  digital_input_12 = 140,
  digital_input_13 = 141,
  digital_input_14 = 142,
  digital_input_15 = 143,
  digital_input_16 = 144,
  bit_0_digital_input_ctrl_word = 160,
  bit_1_digital_input_ctrl_word = 161,
  bit_2_digital_input_ctrl_word = 162,
  bit_3_digital_input_ctrl_word = 163,
  bit_4_digital_input_ctrl_word = 164,
  bit_5_digital_input_ctrl_word = 165,
  bit_6_digital_input_ctrl_word = 166,
  bit_7_digital_input_ctrl_word = 167,
  bit_8_digital_input_ctrl_word = 168,
  bit_9_digital_input_ctrl_word = 169,
  bit10_digital_input_ctrl_word = 170,
  bit11_digital_input_ctrl_word = 171,
  bit12_digital_input_ctrl_word = 172,
  bit13_digital_input_ctrl_word = 173,
  bit14_digital_input_ctrl_word = 174,
  bit15_digital_input_ctrl_word = 175,
  bit_0_modbus_ctrl_word = 176,
  bit_1_modbus_ctrl_word = 177,
  bit_2_modbus_ctrl_word = 178,
  bit_3_modbus_ctrl_word = 179,
  bit_4_modbus_ctrl_word = 180,
  bit_5_modbus_ctrl_word = 181,
  bit_6_modbus_ctrl_word = 182,
  bit_7_modbus_ctrl_word = 183,
  bit_8_modbus_ctrl_word = 184,
  bit_9_modbus_ctrl_word = 185,
  bit_10_modbus_ctrl_word = 186,
  bit_11_modbus_ctrl_word = 187,
  bit_12_modbus_ctrl_word = 188,
  bit_13_modbus_ctrl_word = 189,
  bit_14_modbus_ctrl_word = 190,
  bit_15_modbus_ctrl_word = 191,
  bit_0_canopen_ctrl_word = 192,
  bit_1_canopen_ctrl_word = 193,
  bit_2_canopen_ctrl_word = 194,
  bit_3_canopen_ctrl_word = 195,
  bit_4_canopen_ctrl_word = 196,
  bit_5_canopen_ctrl_word = 197,
  bit_6_canopen_ctrl_word = 198,
  bit_7_canopen_ctrl_word = 199,
  bit_8_canopen_ctrl_word = 200,
  bit_9_canopen_ctrl_word = 201,
  bit_10_canopen_ctrl_word = 202,
  bit_11_canopen_ctrl_word = 203,
  bit_12_canopen_ctrl_word = 204,
  bit_13_canopen_ctrl_word = 205,
  bit_14_canopen_ctrl_word = 206,
  bit_15_canopen_ctrl_word = 207,
  bit_0_com_module_ctrl_word = 208,
  bit_1_com_module_ctrl_word = 209,
  bit_2_com_module_ctrl_word = 210,
  bit_3_com_module_ctrl_word = 211,
  bit_4_com_module_ctrl_word = 212,
  bit_5_com_module_ctrl_word = 213,
  bit_6_com_module_ctrl_word = 214,
  bit_7_com_module_ctrl_word = 215,
  bit_8_com_module_ctrl_word = 216,
  bit_9_com_module_ctrl_word = 217,
  bit_10_com_module_ctrl_word = 218,
  bit_11_com_module_ctrl_word = 219,
  bit_12_com_module_ctrl_word = 220,
  bit_13_com_module_ctrl_word = 221,
  bit_14_com_module_ctrl_word = 222,
  bit_15_com_module_ctrl_word = 223,
  c500 = 240,
  c501 = 241,
  c502 = 242,
  c503 = 243,
  c504 = 244,
  c505 = 245,
  c506 = 246,
  c507 = 247,
  c508 = 248,
  c509 = 249,
  c510 = 250,
  c511 = 251,
  c512 = 252,
  c513 = 253,
  c514 = 254,
  c515 = 255,
  digital_input_di1_low_level = 272,
  digital_input_di2_low_level = 273,
  digital_input_di3_low_level = 274,
  digital_input_di4_low_level = 275,
  digital_input_di5_low_level = 276,
  digital_input_di6_low_level = 277,
  digital_input_di11_low_level = 282,
  digital_input_di12_low_level = 283,
  digital_input_di13_low_level = 284,
  digital_input_di14_low_level = 285,
  digital_input_di15_low_level = 286,
  digital_input_di16_low_level = 287,
  digital_input_di50_high_level = 302,
  digital_input_di51_high_level = 303,
  digital_input_di52_high_level = 304,
  digital_input_di53_high_level = 305,
  digital_input_di54_high_level = 306,
  digital_input_di55_high_level = 307,
  digital_input_di56_high_level = 308,
  digital_input_di57_high_level = 309,
  digital_input_di58_high_level = 310,
  digital_input_di59_high_level = 311,
  digital_input_di50_low_level = 312,
  digital_input_di51_low_level = 313,
  digital_input_di52_low_level = 314,
  digital_input_di53_low_level = 315,
  digital_input_di54_low_level = 316,
  digital_input_di55_low_level = 317,
  digital_input_di56_low_level = 318,
  digital_input_di57_low_level = 319,
  digital_input_di58_low_level = 320,
  digital_input_di59_low_level = 321,
  dc_bus_ripple_warning = 336,
  jockey = 340,
  priming = 341,
  anti_jam_active = 342,
  pipe_fill = 344,
  priming_pump_active = 345,
  dry_run_warning = 346,
  pump_low_flow = 347,
  process_high_flow_warning = 348,
  inlet_pressure_warning = 349,
  outlet_pressure_low_warning = 350,
  outlet_pressure_high_warning = 351,
  pump_cycle_warning = 352,
  anti_jam_warning = 353,
  low_flow_warning = 354,
  low_pressure_warning = 355,
  output_pressure_high_switch_warning = 356,
  jockey_pump_active = 357,
  pump_1_command = 358,
  pump_2_command = 359,
  pump_3_command = 360,
  pump_4_command = 361,
  pump_5_command = 362,
  pump_6_command = 363,
  multi_pump_available_capacity_warning = 364,
  lead_pump_not_available = 365,
  high_level_warning = 366,
  low_level_warning = 367,
  level_switch_warning = 368,
  multipump_device_warning = 369,
  multi_pump_master_activated = 370,
  temperature_sensor_ai2_warning = 475,
  temperature_sensor_ai3_warning = 476,
  temperature_sensor_ai4_warning = 477,
  temperature_sensor_ai5_warning = 478,
  customer_warning_5 = 484,
  cabinet_fan_command = 488,
  circuit_breaker_start_pulse = 489,
  circuit_breaker_stop_pulse = 490,
  power_consumption_warning = 491,
  warning_group_4 = 492,
  warning_group_5 = 493,
  fallback_speed = 494,
  speed_maintained = 495,
  per_type_of_stop = 496,
  life_cycle_warning_1 = 497,
  life_cycle_warning_2 = 498,
  ai2_thermal_sensor_warning = 499,
  ai3_thermal_sensor_warning = 500,
  ai4_thermal_sensor_warning = 501,
  ai5_thermal_sensor_warning = 502,
  ai5_4_20_loss_warning = 503,
  fan_counter_warning = 504,
  fan_feedback_warning = 505,
  power_high_threshold = 506,
  power_low_threshold = 507,
  customer_warning_1 = 508,
  customer_warning_2 = 509,
  customer_warning_3 = 510,
  customer_warning_4 = 511,
};
using assignment_R1 =
    setting<ecx::index_t{ 0x2014, 0x02 }, "AO1", "AQ1 assignment", atv320_psl_e, atv320_psl_e::not_assigned>;

// ATV320 specific data type and multiplier for unit
using decifrequency = mp_units::quantity<mp_units::si::deci<mp_units::si::hertz>, uint16_t>;
using decawatt = mp_units::quantity<mp_units::si::deca<mp_units::si::watt>, uint16_t>;
using atv_deciampere_rep = mp_units::quantity<mp_units::si::deci<mp_units::si::ampere>, uint16_t>;

using namespace mp_units::si::unit_symbols;

// Units 0.01 KW / 10W
using nominal_motor_power_NPR = setting<ecx::index_t{ 0x2042, 0x0E }, "NPR", "Nominal motor power", decawatt, 15 * hW>;
using nominal_motor_voltage_UNS = setting<ecx::index_t{ 0x2042, 0x02 },
                                          "UNS",
                                          "Nominal motor voltage",
                                          mp_units::quantity<mp_units::si::volt, uint16_t>,
                                          400 * V>;

using nominal_motor_frequency_FRS =
    setting<ecx::index_t{ 0x2042, 0x03 }, "FRS", "Nominal motor frequency", decifrequency, 500 * dHz>;

using nominal_motor_current_NCR =
    setting<ecx::index_t{ 0x2042, 0x04 }, "NCR", "Nominal motor current", atv_deciampere_rep, 20 * dA>;

using nominal_motor_speed_NSP = setting<ecx::index_t{ 0x2042, 0x05 }, "NSP", "Nominal motor speed", uint16_t, 1500>;

// Units 0.01
using motor_1_cos_phi_COS = setting<ecx::index_t{ 0x2042, 0x07 }, "COS", "Motor 1 cosinus phi", uint16_t, 80>;

using motor_thermal_current_ITH =
    setting<ecx::index_t{ 0x2042, 0x17 }, "ITH", "motor thermal current", atv_deciampere_rep, 20 * dA>;

//  Units 0.1 Hz, Range 10Hz - 500Hz

using max_frequency_TFR = setting<ecx::index_t{ 0x2001, 0x04 }, "TFR", "Max frequency", decifrequency, 800 * dHz>;
using high_speed_HSP = setting<ecx::index_t{ 0x2001, 0x05 }, "HSP", "High speed", decifrequency, 800 * dHz>;
using low_speed_LSP = setting<ecx::index_t{ 0x2001, 0x06 }, "LSP", "Low speed", decifrequency, 200 * dHz>;

using deciseconds = std::chrono::duration<uint16_t, std::deci>;
// 100 = 10 seconds
// 10 = 1 second
using acceleration_ramp_time_ACC = setting<ecx::index_t{ 0x203c, 0x02 }, "ACC", "Acceleration ramp time", deciseconds, 1>;

// 100 = 10 seconds
// 10 = 1 second
using deceleration_ramp_time_DEC = setting<ecx::index_t{ 0x203c, 0x03 }, "DEC", "Deceleration time ramp", deciseconds, 1>;

namespace detail {
struct speed {
  decifrequency value{ 0 * dHz };
  bool reverse{ false };
  constexpr auto operator==(speed const& other) const noexcept -> bool = default;
};
constexpr auto percentage_to_deci_freq(mp_units::quantity<mp_units::percent, double> percentage,
                                       [[maybe_unused]] low_speed_LSP min_freq,
                                       [[maybe_unused]] high_speed_HSP max_freq) noexcept -> speed {
  if (mp_units::abs(percentage) < 1 * mp_units::percent) {
    return { .value = 0 * dHz, .reverse = false };
  }
  bool reverse = percentage.numerical_value_ <= -1;
  mp_units::Quantity auto mapped{ ec::util::map(mp_units::abs(percentage), (1.0 * mp_units::percent),
                                                (100.0 * mp_units::percent), min_freq.value, max_freq.value) };
  return { .value = mapped, .reverse = reverse };
}
// gcc only supports constexpr std::abs and there is no feature flag
#ifndef __clang__
// stop test
static_assert(percentage_to_deci_freq(0 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) == speed{ .value = 0 * dHz, .reverse = false });
// min test forward
static_assert(percentage_to_deci_freq(1 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) ==
              speed{ .value = 200 * dHz, .reverse = false });
// max test forward
static_assert(percentage_to_deci_freq(100 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) ==
              speed{ .value = 500 * dHz, .reverse = false });
// 50% test forward
static_assert(percentage_to_deci_freq(50 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) ==
              speed{ .value = 348 * dHz, .reverse = false });
// max test reverse
static_assert(percentage_to_deci_freq(-100 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) == speed{ .value = 500 * dHz, .reverse = true });
// min test reverse
static_assert(percentage_to_deci_freq(-1 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) == speed{ .value = 200 * dHz, .reverse = true });
// 50% test reverse
static_assert(percentage_to_deci_freq(-50 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) == speed{ .value = 348 * dHz, .reverse = true });
// outside bounds reverse
static_assert(percentage_to_deci_freq(-10000 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) == speed{ .value = 500 * dHz, .reverse = true });
// outside bounds forward
static_assert(percentage_to_deci_freq(10000 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) ==
              speed{ .value = 500 * dHz, .reverse = false });
#endif
}  // namespace detail

template <typename manager_client_type>
class atv320 final : public base {
public:
  static constexpr uint32_t vendor_id = 0x0800005a;
  static constexpr uint32_t product_code = 0x389;

  struct atv_config {
    nominal_motor_power_NPR nominal_motor_power;
    nominal_motor_voltage_UNS nominal_motor_voltage;
    nominal_motor_current_NCR nominal_motor_current;
    nominal_motor_frequency_FRS nominal_motor_frequency;
    nominal_motor_speed_NSP nominal_motor_speed;
    max_frequency_TFR max_frequency;
    motor_thermal_current_ITH motor_thermal_current;
    high_speed_HSP high_speed;
    low_speed_LSP low_speed;
    motor_1_cos_phi_COS motor_1_cos_phi;
    acceleration_ramp_time_ACC acceleration;
    deceleration_ramp_time_DEC deceleration;
    // write a glaze json converter
    struct glaze {
      using T = atv_config;
      static constexpr auto value = glz::object("nominal_motor_power",
                                                &T::nominal_motor_power,
                                                "nominal_motor_voltage",
                                                &T::nominal_motor_voltage,
                                                "nominal_motor_current",
                                                &T::nominal_motor_current,
                                                "nominal_motor_frequency",
                                                &T::nominal_motor_frequency,
                                                "nominal_motor_speed",
                                                &T::nominal_motor_speed,
                                                "max_frequency",
                                                &T::max_frequency,
                                                "motor_thermal_current",
                                                &T::motor_thermal_current,
                                                "high_speed",
                                                &T::high_speed,
                                                "low_speed",
                                                &T::low_speed,
                                                "cos_phi",
                                                &T::motor_1_cos_phi,
                                                "acceleration",
                                                &T::acceleration,
                                                "deceleration",
                                                &T::deceleration);
      static constexpr std::string_view name{ "atv320" };
    };
    auto operator==(const atv_config&) const noexcept -> bool = default;
  };

  using config_t = tfc::confman::config<confman::observable<atv_config>>;

  explicit atv320(boost::asio::io_context& ctx, manager_client_type& client, uint16_t slave_index)
      : base(slave_index),
        state_transmitter_(ctx, client, fmt::format("atv320.s{}.state", slave_index), "Current CIA402 state"),
        command_transmitter_(ctx, client, fmt::format("atv320.s{}.command", slave_index), "Current CIA402 command"),
        run_(ctx,
             client,
             fmt::format("atv320.s{}.run", slave_index),
             "Turn on motor",
             [this](bool value) { running_ = value; }),
        ratio_(ctx,
               client,
               fmt::format("atv320.s{}.ratio", slave_index),
               "Speed ratio.\n100% is max freq.\n1%, is min freq.\n(-1, 1)% is stopped.\n-100% is reverse max freq.\n-1% is "
               "reverse min freq.",
               [this](double value) {
                 reference_frequency_ = detail::percentage_to_deci_freq(
                     value * mp_units::percent, config_.value().value().low_speed, config_.value().value().high_speed);
               }),
        frequency_transmit_(ctx, client, fmt::format("atv320.s{}.in.freq", slave_index), "Current Frequency"),
        config_{ ctx, fmt::format("atv320_i{}", slave_index) } {
    config_->observe([this](auto&, auto&) {
      logger_.warn(
          "Live motor configuration unsupported, config change registered will be applied next ethercat master restart");
    });
    for (size_t i = 0; i < 6; i++) {
      di_transmitters_.emplace_back(
          tfc::ipc::bool_signal(ctx, client, fmt::format("atv320.s{}.in{}", slave_index, i), "Digital Input"));
    }
    for (size_t i = 0; i < 2; i++) {
      ai_transmitters_.emplace_back(
          tfc::ipc::int_signal(ctx, client, fmt::format("atv320.s{}.in{}", slave_index, i), "Analog input"));
    }
  }

  struct input_t {
    tfc::ec::cia_402::status_word status_word{};
    uint16_t frequency{};
    uint16_t current{};
    uint16_t digital_inputs{};
    std::array<int16_t, 2> analog_inputs{};
  };
  static_assert(sizeof(input_t) == 12);
  struct output_t {
    cia_402::control_word control{};
    decifrequency frequency{};
    uint16_t digital_outputs{};
  };
  static_assert(sizeof(output_t) == 6);

  static_assert(sizeof(input_t) == 12);
  static_assert(sizeof(output_t) == 6);

  auto process_data(std::span<std::byte> input, std::span<std::byte> output) noexcept -> void final {
    // All registers in the ATV320 ar uint16, create a pointer to this memory
    // With the same size
    // these sizes are in bytes not uint16_t
    if (input.size() != sizeof(input_t) || output.size() != sizeof(output_t))
      return;
    const input_t* in = std::launder(reinterpret_cast<input_t*>(input.data()));
    output_t* out = std::launder(reinterpret_cast<output_t*>(output.data()));

    auto state = in->status_word.parse_state();
    if (state != last_state_) {
      state_transmitter_.async_send(cia_402::to_string(state), [this](auto&& PH1, size_t bytes_transfered) {
        async_send_callback(std::forward<decltype(PH1)>(PH1), bytes_transfered);
      });
      last_state_ = state;
    }
    std::bitset<6> const value(in->digital_inputs);
    for (size_t i = 0; i < 6; i++) {
      if (value.test(i) != last_bool_values_.test(i)) {
        di_transmitters_[i].async_send(value.test(i), [this](auto&& PH1, size_t bytes_transfered) {
          async_send_callback(std::forward<decltype(PH1)>(PH1), bytes_transfered);
        });
      }
    }
    last_bool_values_ = value;

    for (size_t i = 0; i < 2; i++) {
      if (last_analog_inputs_[i] != in->analog_inputs[i]) {
        ai_transmitters_[i].async_send(in->analog_inputs[i], [this](auto&& PH1, size_t const bytes_transfered) {
          async_send_callback(std::forward<decltype(PH1)>(PH1), bytes_transfered);
        });
      }
      last_analog_inputs_[i] = in->analog_inputs[i];
    }
    auto frequency = static_cast<int16_t>(in->frequency);
    if (last_frequency_ != frequency) {
      frequency_transmit_.async_send(static_cast<double>(frequency) / 10, [this](auto&& PH1, size_t const bytes_transfered) {
        async_send_callback(std::forward<decltype(PH1)>(PH1), bytes_transfered);
      });
    }

    last_frequency_ = frequency;
    using tfc::ec::cia_402::commands_e;
    using tfc::ec::cia_402::states_e;
    bool const quick_stop = !running_ || reference_frequency_.value == 0 * dHz;
    auto command = tfc::ec::cia_402::transition(state, quick_stop);

    if (cia_402::to_string(command) != last_command_) {
      command_transmitter_.async_send(cia_402::to_string(command), [this](auto&& PH1, size_t const bytes_transfered) {
        async_send_callback(std::forward<decltype(PH1)>(PH1), bytes_transfered);
      });
    }

    out->control = cia_402::control_word::from_uint(std::to_underlying(command));
    // Reverse bit is bit 11 of control word from
    // https://iportal2.schneider-electric.com/Contents/docs/SQD-ATV320U11N4C_USER%20GUIDE.PDF
    out->control.reserved_1 = reference_frequency_.reverse;
    out->frequency = running_ ? reference_frequency_.value : 0 * mp_units::quantity<mp_units::si::hertz, uint16_t>{};
  }

  auto async_send_callback(std::error_code const& error, size_t) -> void {
    if (error) {
      logger_.error("Ethercat ATV320 error: {}", error.message());
    }
  }

  auto setup() -> int final {
    // Set PDO variables
    // Clean rx and tx prod assign
    sdo_write<uint8_t>(ecx::rx_pdo_assign<0x00>, 0);
    sdo_write<uint8_t>(ecx::tx_pdo_assign<0x00>, 0);
    // Zero the size
    sdo_write<uint8_t>(ecx::tx_pdo_mapping<0x00>, 0);
    sdo_write<uint32_t>(ecx::tx_pdo_mapping<0x01>, 0x60410010);  // ETA  - STATUS WORD
    sdo_write<uint32_t>(ecx::tx_pdo_mapping<0x02>, 0x20020310);  // RFR  - CURRENT SPEED HZ
    sdo_write<uint32_t>(ecx::tx_pdo_mapping<0x03>, 0x20020510);  // LCR  - CURRENT USAGE ( A
    sdo_write<uint32_t>(ecx::tx_pdo_mapping<0x04>, 0x20160310);  // 1LIR - DI1-DI6
    sdo_write<uint32_t>(ecx::tx_pdo_mapping<0x05>, 0x20162B10);  // AI1C - Physical value AI1
    sdo_write<uint32_t>(ecx::tx_pdo_mapping<0x06>, 0x20162D10);  // AI3C - Physical value AI3
    sdo_write<uint8_t>(ecx::tx_pdo_mapping<0x00>, 6);

    // Zero the size
    sdo_write<uint8_t>(ecx::rx_pdo_mapping<0x00>, 0);
    // Assign tx variables
    sdo_write<uint32_t>(ecx::rx_pdo_mapping<0x01>,
                        ecx::make_mapping_value<cia_402::control_word>());  // CMD - CONTROL WORD
    sdo_write<uint32_t>(ecx::rx_pdo_mapping<0x02>, 0x20370310);             // LFR - REFERENCE SPEED HZ
    sdo_write<uint32_t>(
        ecx::rx_pdo_mapping<0x03>,
        0x20160D10);  // OL1R - Logic outputs states ( bit0: Relay 1, bit1: Relay 2, bit3 - bit7: unknown, bit8: DQ1 )
    sdo_write<uint32_t>(ecx::rx_pdo_mapping<0x03>, 0x20164810);  // AO1C - AQ1 physical value

    // Set tx size
    sdo_write<uint8_t>(ecx::rx_pdo_mapping<0x00>, 3);

    // // Assign pdo's to mappings
    sdo_write<uint16_t>(ecx::rx_pdo_assign<0x01>, ecx::rx_pdo_mapping<>.first);
    sdo_write<uint8_t>(ecx::rx_pdo_assign<0x00>, 1);

    sdo_write<uint16_t>(ecx::tx_pdo_assign<0x01>, ecx::tx_pdo_mapping<>.first);
    sdo_write<uint8_t>(ecx::tx_pdo_assign<0x00>, 1);

    sdo_write(configuration_reference_frequency_1_FR1{ .value = atv320_psa_e::reference_frequency_via_com_module });
    // Clear internal ATV Functionality for outputs and inputs
    sdo_write(assignment_R1{ .value = atv320_psl_e::not_assigned });
    sdo_write(assignment_AQ1{ .value = atv320_psa_e::not_configured });
    sdo_write(configuration_of_AI1{ .value = atv320_aiot_e::current });
    sdo_write(configuration_of_AI3{ .value = atv320_aiot_e::current });
    sdo_write(analog_input_3_range{ .value = atv320_aiol_e::positive_only });

    // test writing alias address - this does not seem to work. Direct eeprom writing also possible working.
    // sdo_write<uint16_t>({ 0x2024, 0x92 }, 1337);  // 2 - Current

    // assign motor parameters from config. For now just setup the test motor

    sdo_write(config_->value().nominal_motor_power);
    sdo_write(config_->value().nominal_motor_voltage);
    sdo_write(config_->value().nominal_motor_current);
    sdo_write(config_->value().nominal_motor_frequency);
    sdo_write(config_->value().nominal_motor_speed);
    sdo_write(config_->value().max_frequency);
    sdo_write(config_->value().motor_thermal_current);
    sdo_write(config_->value().high_speed);
    sdo_write(config_->value().low_speed);
    sdo_write(config_->value().motor_1_cos_phi);
    sdo_write(config_->value().acceleration);
    sdo_write(config_->value().deceleration);
    return 1;
  }

private:
  std::array<int16_t, 2> last_analog_inputs_;
  std::vector<tfc::ipc::int_signal> ai_transmitters_;
  std::bitset<6> last_bool_values_;
  std::vector<tfc::ipc::bool_signal> di_transmitters_;
  cia_402::states_e last_state_;
  tfc::ipc::string_signal state_transmitter_;
  std::string last_command_;
  tfc::ipc::string_signal command_transmitter_;
  bool running_{};
  tfc::ipc::bool_slot run_;
  detail::speed reference_frequency_{ .value = 0 * dHz, .reverse = false };
  tfc::ipc::double_slot ratio_;
  int16_t last_frequency_;
  tfc::ipc::double_signal frequency_transmit_;
  config_t config_;
};
}  // namespace tfc::ec::devices::schneider
