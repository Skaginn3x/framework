#pragma once

// Useful resources
// https://download.schneider-electric.com/files?p_enDocType=User+guide&p_File_Name=0198441113868_06.pdf&p_Doc_Ref=0198441113868-EN
// https://www.elmomc.com/capabilities/motion-control/ethercat-communication/ethercat-coe-ds402-support/
// https://iportal2.schneider-electric.com/Contents/docs/SQD-VW3M5101R200_USER%20GUIDE.PDF

#include <boost/asio/io_context.hpp>
#include <glaze/glaze.hpp>

#include <tfc/confman.hpp>
#include <tfc/stx/basic_fixed_string.hpp>

#include "lxm32m/glaze_meta.hpp"
#include "lxm32m/pdos.hpp"
#include "lxm32m/settings.hpp"
#include "tfc/cia/402.hpp"
#include "tfc/ec/devices/base.hpp"

namespace tfc::ec::devices::schneider::lxm32m {

namespace asio = boost::asio;

struct input_pdo {
  cia_402::status_word status{};
};

struct output_pdo {
  cia_402::commands_e command{};
  settings::operation_mode mode{};
  settings::operation_mode placeholder{};
  settings::target_velocity velocity{};
  settings::acceleration_ramp acc_ramp{};
  settings::deceleration_ramp dec_ramp{};
};

struct config {
  settings::operation_mode mode{ .value = settings::operation_mode_e::profile_velocity };
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

    if (in->status.warning) { // error class 0
      return on_error();
    }
    if (in->status.application_specific_1) { // error class 1
      return on_error();
    }

    [[maybe_unused]] auto state = in->status.parse_state();

    out->command = cia_402::transition(state, false);

    out->velocity.value = 100;  // todo
  }

  void on_error() {
    // todo sdo_read _WarnLatched
    // todo sdo_read _SigLatched
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
    base::sdo_write(ecx::rx_pdo_mapping<0x02>, ecx::make_mapping_value<settings::operation_mode>());
    base::sdo_write(ecx::rx_pdo_mapping<0x03>, ecx::make_mapping_value<settings::operation_mode>());
    base::sdo_write(ecx::rx_pdo_mapping<0x04>, ecx::make_mapping_value<settings::target_velocity>());
    base::sdo_write(ecx::rx_pdo_mapping<0x05>, ecx::make_mapping_value<settings::acceleration_ramp>());
    base::sdo_write(ecx::rx_pdo_mapping<0x06>, ecx::make_mapping_value<settings::deceleration_ramp>());
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
    sdo_write(
        settings::compatibility_for_synchronous_motor{ .value = settings::compatibility_for_synchronous_motor_e::off });
    sdo_write(settings::modulo_enable{ .value = settings::modulo_enable_e::off });
    sdo_write(settings::quick_stop_option{ .value = settings::quick_stop_option_e::deceleration_ramp });
    sdo_write(settings::response_to_active_limit{ .value = settings::response_to_active_limit_e::error });
    sdo_write(settings::position_scaling_denom{ .value = 16384 });
    sdo_write(settings::position_scaling_num{ .value = 1 });
    sdo_write(settings::velocity_feed_forward_control_1{ .value = 1000 });
    sdo_write(settings::velocity_feed_forward_control_2{ .value = 1000 });
    sdo_write(settings::operation_mode{ .value = settings::operation_mode_e::profile_velocity });
    sdo_write(settings::input_shift_time{});

    return 1;
  }

  tfc::confman::config<config> config_;
};

}  // namespace tfc::ec::devices::schneider::lxm32m
