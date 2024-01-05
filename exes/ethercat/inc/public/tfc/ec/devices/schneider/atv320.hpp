#pragma once

#include <fmt/format.h>
#include <mp-units/math.h>
#include <mp-units/systems/isq/isq.h>
#include <mp-units/systems/si/si.h>
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

#include <tfc/ec/devices/schneider/atv320/dbus-iface.hpp>
#include <tfc/ec/devices/schneider/atv320/enums.hpp>
#include <tfc/ec/devices/schneider/atv320/pdo.hpp>
#include <tfc/ec/devices/schneider/atv320/settings.hpp>
#include <tfc/ec/devices/schneider/atv320/speedratio.hpp>

namespace tfc::ec::devices::schneider::atv320 {
using tfc::ec::util::setting;

namespace details {
template <typename signal_t, typename variable_t, typename logger_t>
inline variable_t async_send_if_new(signal_t& signal,
                                    const variable_t& old_var,
                                    const variable_t& new_var,
                                    logger_t& logger) {
  // clang-format off
  PRAGMA_CLANG_WARNING_PUSH_OFF(-Wfloat-equal)
  if (old_var != new_var) {
  PRAGMA_CLANG_WARNING_POP
    // clang-format on
    signal.async_send(new_var, [&logger](const std::error_code& err, size_t) {
      if (err) {
        logger.error("ATV failed to send");
      }
    });
  }
  return new_var;
}
};  // namespace details

template <typename manager_client_t>
class device final : public base {
public:
  static constexpr uint32_t vendor_id = 0x0800005a;
  static constexpr uint32_t product_code = 0x389;
  static constexpr size_t atv320_di_count = 6;

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
    // It looks like acc and dec configure the time from 0 to nominal
    // f.e. a configuration of 3.5 seconds for a nominal drive of 50Hz
    // takes approx 5 seconds to reach 80Hz
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

  explicit device(std::shared_ptr<sdbusplus::asio::connection> connection, manager_client_t& client, uint16_t slave_index)
      : base(slave_index), run_(connection->get_io_context(),
                                client,
                                fmt::format("atv320.s{}.run", slave_index),
                                "Turn on motor",
                                [this](bool value) { ipc_running_ = value; }),
        ratio_(connection->get_io_context(),
               client,
               fmt::format("atv320.s{}.ratio", slave_index),
               "Speed ratio.\n100% is max freq.\n1%, is min freq.\n(-1, 1)% is stopped.\n-100% is reverse max freq.\n-1% is "
               "reverse min freq.",
               [this](double value) {
                 reference_frequency_ = detail::percentage_to_deci_freq(
                     value * mp_units::percent, config_.value().value().low_speed, config_.value().value().high_speed);
               }),
        frequency_transmit_(connection->get_io_context(),
                            client,
                            fmt::format("atv320.s{}.in.freq", slave_index),
                            "Current Frequency"),
        hmis_transmitter_(connection->get_io_context(), client, fmt::format("atv320.s{}.hmis", slave_index), "HMI state"),
        config_{ connection->get_io_context(), fmt::format("atv320_i{}", slave_index) },
        dbus_iface_(connection, slave_index, client) {
    config_->observe([this](auto&, auto&) {
      logger_.warn(
          "Live motor configuration unsupported, config change registered will be applied next ethercat master restart");
    });
    for (size_t i = 0; i < atv320_di_count; i++) {
      di_transmitters_.emplace_back(tfc::ipc::bool_signal(connection->get_io_context(), client,
                                                          fmt::format("atv320.s{}.in{}", slave_index, i), "Digital Input"));
    }
    // TODO: Design a sane method for transmitting analog signals in event based enviorments
    // for (size_t i = 0; i < 2; i++) {
    //   ai_transmitters_.emplace_back(
    //       tfc::ipc::int_signal(ctx, client, fmt::format("atv320.s{}.in{}", slave_index, i), "Analog input"));
    // }
  }

  // Update signals of the current status of the drive
  void transmit_status(const input_t& input) {
    std::bitset<atv320_di_count> const value(input.digital_inputs);
    for (size_t i = 0; i < atv320_di_count; i++) {
      last_bool_values_.set(
          i, details::async_send_if_new(di_transmitters_[i], last_bool_values_.test(i), value.test(i), logger_));
    }
    last_hmis_ = details::async_send_if_new(hmis_transmitter_, last_hmis_, input.drive_state, logger_);
    double frequency = static_cast<double>(input.frequency.numerical_value_is_an_implementation_detail_) / 10.0;
    last_frequency_ = details::async_send_if_new(frequency_transmit_, last_frequency_, frequency, logger_);
  }

  auto process_data(std::span<std::byte> input, std::span<std::byte> output) noexcept -> void final {
    // All registers in the ATV320 ar uint16, create a pointer to this memory
    // With the same size
    // these sizes are in bytes not uint16_t
    if (input.size() != sizeof(input_t) || output.size() != sizeof(output_t)) {
      return;
    }
    const input_t* in = std::launder(reinterpret_cast<input_t*>(input.data()));
    output_t* out = std::launder(reinterpret_cast<output_t*>(output.data()));

    transmit_status(*in);

    dbus_iface_.update_status(*in);
    if (!dbus_iface_.has_peer()) {
      // Quick stop if frequncy set to 0
      const bool quick_stop = reference_frequency_.value == 0 * dHz;

      auto state = in->status_word.parse_state();
      out->control = cia_402::transition(state, ipc_running_, quick_stop, false);

      // Reverse bit is bit 11 of control word from
      // https://iportal2.schneider-electric.com/Contents/docs/SQD-ATV320U11N4C_USER%20GUIDE.PDF
      out->control.reserved_1 = reference_frequency_.reverse;
      out->frequency = reference_frequency_.value;
    } else {
      auto freq = detail::percentage_to_deci_freq(dbus_iface_.speed_ratio(), config_->value().low_speed,
                                                  config_->value().high_speed);
      out->frequency = freq.value;
      out->control.reserved_1 = freq.reverse;
      out->control = dbus_iface_.ctrl();

      // Set running to false. Will need to be set high before the motor starts on ipc
      // after dbus disconnect
      ipc_running_ = false;
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
  std::bitset<atv320_di_count> last_bool_values_;
  std::vector<tfc::ipc::bool_signal> di_transmitters_;
  bool ipc_running_{};
  tfc::ipc::bool_slot run_;
  detail::speed reference_frequency_{ .value = 0 * dHz, .reverse = false };
  tfc::ipc::double_slot ratio_;
  double last_frequency_;
  tfc::ipc::double_signal frequency_transmit_;
  uint16_t last_hmis_{};
  tfc::ipc::uint_signal hmis_transmitter_;
  config_t config_;
  dbus_iface<manager_client_t> dbus_iface_;
};
}  // namespace tfc::ec::devices::schneider::atv320
