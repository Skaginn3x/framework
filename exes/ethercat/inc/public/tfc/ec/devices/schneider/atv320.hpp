#pragma once

#include <fmt/format.h>
#include <mp-units/math.h>
#include <mp-units/systems/isq/isq.h>
#include <mp-units/systems/si/si.h>
#include <bitset>
#include <memory>
#include <optional>

#include "tfc/cia/402.hpp"
#include "tfc/confman.hpp"
#include "tfc/confman/observable.hpp"
#include "tfc/ec/devices/base.hpp"
#include "tfc/ec/devices/util.hpp"
#include "tfc/ec/soem_interface.hpp"
#include "tfc/ipc.hpp"
#include "tfc/utils/units_glaze_meta.hpp"

#include <tfc/ec/devices/schneider/atv320/enums.hpp>
#include <tfc/ec/devices/schneider/atv320/settings.hpp>
#include <tfc/ec/devices/schneider/atv320/speedratio.hpp>

namespace tfc::ec::devices::schneider::atv320 {
using tfc::ec::util::setting;

template <typename manager_client_type>
class device final : public base {
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

  explicit device(boost::asio::io_context& ctx, manager_client_type& client, uint16_t slave_index)
      : base(slave_index), run_(ctx,
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
        hmis_transmitter_(ctx, client, fmt::format("atv320.s{}.hmis", slave_index), "HMI state"),
        config_{ ctx, fmt::format("atv320_i{}", slave_index) } {
    config_->observe([this](auto&, auto&) {
      logger_.warn(
          "Live motor configuration unsupported, config change registered will be applied next ethercat master restart");
    });
    for (size_t i = 0; i < 6; i++) {
      di_transmitters_.emplace_back(
          tfc::ipc::bool_signal(ctx, client, fmt::format("atv320.s{}.in{}", slave_index, i), "Digital Input"));
    }
    // TODO: Design a sane method for transmitting analog signals in event based enviorments
    // for (size_t i = 0; i < 2; i++) {
    //   ai_transmitters_.emplace_back(
    //       tfc::ipc::int_signal(ctx, client, fmt::format("atv320.s{}.in{}", slave_index, i), "Analog input"));
    // }
  }

  struct input_t {
    tfc::ec::cia_402::status_word status_word{};
    uint16_t frequency{};
    uint16_t current{};
    uint16_t digital_inputs{};
    int16_t analog_input{};
    uint16_t drive_state{};
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
    if (input.size() != sizeof(input_t) || output.size() != sizeof(output_t)) {
      return;
    }
    const input_t* in = std::launder(reinterpret_cast<input_t*>(input.data()));
    output_t* out = std::launder(reinterpret_cast<output_t*>(output.data()));

    std::bitset<6> const value(in->digital_inputs);
    for (size_t i = 0; i < 6; i++) {
      if (value.test(i) != last_bool_values_.test(i)) {
        di_transmitters_[i].async_send(value.test(i), [this](auto&& PH1, size_t bytes_transfered) {
          async_send_callback(std::forward<decltype(PH1)>(PH1), bytes_transfered);
        });
      }
    }
    last_bool_values_ = value;

    if (in->drive_state != last_hmis_) {
      hmis_transmitter_.async_send(in->drive_state, [this](auto&& PH1, size_t bytes_transfered) {
        async_send_callback(std::forward<decltype(PH1)>(PH1), bytes_transfered);
      });
      last_hmis_ = in->drive_state;
    }

    // TODO: Design a sane method for transmitting analog signals in event based environments
    // for (size_t i = 0; i < 2; i++) {
    //   if (last_analog_inputs_[i] != in->analog_inputs[i]) {
    //     ai_transmitters_[i].async_send(in->analog_inputs[i], [this](auto&& PH1, size_t const bytes_transfered) {
    //       async_send_callback(std::forward<decltype(PH1)>(PH1), bytes_transfered);
    //     });
    //   }
    //   last_analog_inputs_[i] = in->analog_inputs[i];
    // }
    auto frequency = static_cast<int16_t>(in->frequency);
    if (last_frequency_ != frequency) {
      frequency_transmit_.async_send(static_cast<double>(frequency) / 10, [this](auto&& PH1, size_t const bytes_transfered) {
        async_send_callback(std::forward<decltype(PH1)>(PH1), bytes_transfered);
      });
    }

    last_frequency_ = frequency;
    using tfc::ec::cia_402::states_e;
    bool const quick_stop = !running_ || reference_frequency_.value == 0 * dHz;

    auto state = in->status_word.parse_state();
    auto command = tfc::ec::cia_402::transition(state, quick_stop);

    out->control = cia_402::control_word::from_uint(command);
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
    sdo_write<uint32_t>(ecx::tx_pdo_mapping<0x06>, 0x20022910);  // HMIS - Drive state
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

    sdo_write(configuration_reference_frequency_1_FR1{ .value = psa_e::reference_frequency_via_com_module });
    // Clear internal ATV Functionality for outputs and inputs
    sdo_write(assignment_R1{ .value = psl_e::not_assigned });
    sdo_write(assignment_AQ1{ .value = psa_e::not_configured });
    sdo_write(configuration_of_AI1{ .value = aiot_e::current });
    sdo_write(configuration_of_AI3{ .value = aiot_e::current });
    sdo_write(analog_input_3_range{ .value = aiol_e::positive_only });

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
  // int16_t last_analog_inputs_;
  // tfc::ipc::int_signal ai_transmitter_;
  std::bitset<6> last_bool_values_;
  std::vector<tfc::ipc::bool_signal> di_transmitters_;
  bool running_{};
  tfc::ipc::bool_slot run_;
  detail::speed reference_frequency_{ .value = 0 * dHz, .reverse = false };
  tfc::ipc::double_slot ratio_;
  int16_t last_frequency_;
  tfc::ipc::double_signal frequency_transmit_;
  uint16_t last_hmis_{};
  tfc::ipc::uint_signal hmis_transmitter_;
  config_t config_;
};
}  // namespace tfc::ec::devices::schneider::atv320
