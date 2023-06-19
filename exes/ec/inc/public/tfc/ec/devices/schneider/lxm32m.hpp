#pragma once

// Useful resources
// https://download.schneider-electric.com/files?p_enDocType=User+guide&p_File_Name=0198441113868_06.pdf&p_Doc_Ref=0198441113868-EN
// https://www.elmomc.com/capabilities/motion-control/ethercat-communication/ethercat-coe-ds402-support/

#include <boost/asio/io_context.hpp>
#include <glaze/glaze.hpp>

#include <tfc/confman.hpp>
#include <tfc/stx/basic_fixed_string.hpp>

#include "lxm32m/pdos.hpp"
#include "tfc/cia/402.hpp"
#include "tfc/ec/devices/base.hpp"

namespace tfc::ec::devices::schneider::lxm32m {



}  // namespace tfc::ec::devices::schneider

namespace glz {

template <>
struct meta<tfc::ec::devices::schneider::lxm32m::operation_mode_e> {
  using enum tfc::ec::devices::schneider::lxm32m::operation_mode_e;
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

namespace tfc::ec::devices::schneider::lxm32m {

namespace asio = boost::asio;

struct input_pdo {
  cia_402::status_word status{};
};

struct output_pdo {
  cia_402::commands_e command{};
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
    static constexpr auto name{ "lxm32m" };
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

    out->command = cia_402::transition(state, false);

    out->velocity.value = 100;  // todo
  }

  auto setup() -> int final {
    logger_.trace("Setup of lxm32m");

    // lxm32m supports 4 sets of RxPDOs and TxPDOs
    // and each set can only contain at most 10 parameters


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
