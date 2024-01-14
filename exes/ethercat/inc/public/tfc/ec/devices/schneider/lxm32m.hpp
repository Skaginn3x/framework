#pragma once
// https://download.schneider-electric.com/files?p_enDocType=User+guide&p_File_Name=0198441113868_06.pdf&p_Doc_Ref=0198441113868-EN

#include <boost/asio/io_context.hpp>
#include <glaze/glaze.hpp>

#include <tfc/confman.hpp>
#include <tfc/stx/basic_fixed_string.hpp>

#include "tfc/cia/402.hpp"
#include "tfc/ec/devices/base.hpp"
#include "tfc/ec/devices/util.hpp"

namespace tfc::ec::devices::schneider {

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

}  // namespace tfc::ec::devices::schneider

namespace glz {

template <>
struct meta<tfc::ec::devices::schneider::operation_mode_e> {
  using enum tfc::ec::devices::schneider::operation_mode_e;
  // clang-format off
  static constexpr auto value{ glz::enumerate(
      "manual_or_autotuning", manual_or_autotuning,
      "motion_sequence", motion_sequence,
      "electronic_gear", electronic_gear,
      "jog", jog,
      "reserved", reserved,
      "profile_position", profile_position,
      "profile_velocity", profile_velocity,
      "profile_torque", profile_torque,
      "homing", homing,
      "interpolated_position", interpolated_position,
      "cyclic_synchronous_position", cyclic_synchronous_position,
      "cyclic_synchronous_velocity", cyclic_synchronous_velocity,
      "cyclic_synchronous_torque", cyclic_synchronous_torque
      ) };
  // clang-format on
  static constexpr std::string_view name{ "operation_mode" };
};

}  // namespace glz

namespace tfc::ec::devices::schneider {

namespace asio = boost::asio;

struct input_pdo {
  cia_402::status_word status{};
};

struct output_pdo {
  cia_402::control_word ctrl_word{};
  operation_mode mode{};
  operation_mode placeholder{};
  target_velocity velocity{};
  acceleration_ramp acc_ramp{};
  deceleration_ramp dec_ramp{};
};

struct config {
  operation_mode mode{ .value = operation_mode_e::profile_velocity };
  //  target_velocity target_speed{};
  struct glaze {
    using t = config;
    static constexpr auto value{
      glz::object("operation_mode", &t::mode, "Motor operating mode"
                  //                                              ,"target_velocity", &t::target_speed, "Target velocity mm/s
                  //                                              (requires num and denum to be set properly)"
                  )
    };
    static constexpr std::string_view name{ "lxm32m" };
  };
};

template <typename manager_client_type>
class lxm32m final : public base {
public:
  static constexpr uint32_t vendor_id = 0x800005a;
  static constexpr uint32_t product_code = 0x16440;

  explicit lxm32m([[maybe_unused]] asio::io_context& ctx, [[maybe_unused]] manager_client_type& client, uint16_t slave_index)
      : base(slave_index), config_(ctx, fmt::format("lxm32m.{}", slave_index)) {}

  void process_data(std::span<std::byte> input, std::span<std::byte> output) final {
    [[maybe_unused]] auto* out = std::launder(reinterpret_cast<output_pdo*>(output.data()));
    auto* in = std::launder(reinterpret_cast<input_pdo*>(input.data()));

    [[maybe_unused]] auto state = in->status.parse_state();

    out->ctrl_word = cia_402::transition(state, cia_402::transition_action::run, true);

    out->velocity.value = 100;  // todo
  }

  auto setup() -> int final {
    logger_.trace("Setup of lxm32m");

    base::sdo_write<uint8_t>(ecx::rx_pdo_assign<0x00>, 0);  // write access
    base::sdo_write<uint8_t>(ecx::tx_pdo_assign<0x00>, 0);  // write access

    // adapt RxPDO
    // Subindexes can be written if SI0 is set as 0
    base::sdo_write<uint8_t>(ecx::rx_pdo_mapping<0x00>, 0);

    static_assert(ecx::make_mapping_value<cia_402::control_word>() == 0x60400010);  // move tests to function definition
    base::sdo_write(ecx::rx_pdo_mapping<0x01>, ecx::make_mapping_value<cia_402::control_word>());
    base::sdo_write(ecx::rx_pdo_mapping<0x02>, ecx::make_mapping_value<operation_mode>());
    base::sdo_write(ecx::rx_pdo_mapping<0x03>, ecx::make_mapping_value<operation_mode>());
    base::sdo_write(ecx::rx_pdo_mapping<0x04>, ecx::make_mapping_value<target_velocity>());
    base::sdo_write(ecx::rx_pdo_mapping<0x05>, ecx::make_mapping_value<acceleration_ramp>());
    base::sdo_write(ecx::rx_pdo_mapping<0x06>, ecx::make_mapping_value<deceleration_ramp>());
    base::sdo_write<uint8_t>(ecx::rx_pdo_mapping<0x00>, 6);

    // adapt TxPDO
    // Subindexes can be written if SI0 is set as 0
    base::sdo_write<uint8_t>(ecx::tx_pdo_mapping<0x00>, 0);
    base::sdo_write(ecx::tx_pdo_mapping<0x01>, ecx::make_mapping_value<cia_402::status_word>());
    // todo _DCOMopmd_act is 0x6061 but the example says it is 0x6060
    //    base::sdo_write(ecx::tx_pdo_mapping<0x02>, ecx::make_mapping_value<operation_mode>());
    //    base::sdo_write(ecx::tx_pdo_mapping<0x03>, ecx::make_mapping_value<operation_mode>());
    base::sdo_write<uint8_t>(ecx::tx_pdo_mapping<0x00>, 1);

    // Assign pdo's to mappings
    // todo why is this needed
    base::sdo_write<uint16_t>(ecx::rx_pdo_assign<0x01>, ecx::rx_pdo_mapping<>.first);
    base::sdo_write<uint8_t>(ecx::rx_pdo_assign<0x00>, 1);

    base::sdo_write<uint16_t>(ecx::tx_pdo_assign<0x01>, ecx::tx_pdo_mapping<>.first);
    base::sdo_write<uint8_t>(ecx::tx_pdo_assign<0x00>, 1);

    // setup parameters
    // values from schneider example
    sdo_write(compatibility_for_synchronous_motor{ .value = compatibility_for_synchronous_motor_e::off });
    sdo_write(modulo_enable{ .value = modulo_enable_e::off });
    sdo_write(quick_stop_option{ .value = quick_stop_option_e::deceleration_ramp });
    sdo_write(response_to_active_limit{ .value = response_to_active_limit_e::error });
    sdo_write(position_scaling_denom{ .value = 16384 });
    sdo_write(position_scaling_num{ .value = 1 });
    sdo_write(velocity_feed_forward_control_1{ .value = 1000 });
    sdo_write(velocity_feed_forward_control_2{ .value = 1000 });
    sdo_write(operation_mode{ .value = operation_mode_e::profile_velocity });
    sdo_write(input_shift_time{});

    return 1;
  }

  tfc::confman::config<config> config_;
};

}  // namespace tfc::ec::devices::schneider
