#pragma once

#include <mp-units/math.h>
#include <mp-units/systems/si/si.h>
#include <algorithm>
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
    confman::observable<speedratio_t> default_speedratio{ 1 * speedratio_t::reference };  // Run the motor at LSP by default
    fast_stop_ramp_divider_DCF fast_stop_ramp_divider;
    async_motor_leakage_inductance_LFA async_motor_leakage_inductance;
    async_motor_stator_resistance_RSA async_motor_stator_resistance;
    rotor_time_constant_TRA rotor_time_constant;

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
                                                &T::deceleration,
                                                "default_speedratio",
                                                &T::default_speedratio,
                                                "fast_stop_ramp_divider",
                                                &T::fast_stop_ramp_divider,
                                                "async_motor_leakage_inductance",
                                                &T::async_motor_leakage_inductance,
                                                "async_motor_stator_resistance",
                                                &T::async_motor_stator_resistance,
                                                "rotor_time_constant",
                                                &T::rotor_time_constant,
                                                json::schema{ .minimum = 0, .maximum = 10 });
      static constexpr std::string_view name{ "atv320" };
    };

    auto operator==(const atv_config&) const noexcept -> bool = default;
  };

  using config_t = confman::config<confman::observable<atv_config>>;

  explicit device(std::shared_ptr<sdbusplus::asio::connection> connection, manager_client_t& client, uint16_t slave_index)
      : base(slave_index), ctx_{ connection->get_io_context() }, run_(ctx_,
                                                                      client,
                                                                      fmt::format("atv320.s{}.run", slave_index),
                                                                      "Turn on motor",
                                                                      [this](bool value) { ipc_running_ = value; }),
        ratio_(ctx_,
               client,
               fmt::format("atv320.s{}.ratio", slave_index),
               "Speed ratio.\n100% is max freq.\n1%, is min freq.\n(-1, 1)% is stopped.\n-100% is reverse max freq.\n-1% is "
               "reverse min freq.",
               [this](double value) {
                 reference_frequency_ = detail::percentage_to_deci_freq(
                     value * mp_units::percent, config_.value().value().low_speed, config_.value().value().high_speed);
               }),
        frequency_transmit_(ctx_, client, fmt::format("atv320.s{}.freq", slave_index), "Current Frequency"),
        current_transmit_(ctx_, client, fmt::format("atv320.s{}.current", slave_index), "Current Current"),
        last_error_transmit_(ctx_, client, fmt::format("atv320.s{}.last_error", slave_index), "Last Error [LFT]"),
        hmis_transmitter_(ctx_, client, fmt::format("atv320.s{}.hmis", slave_index), "HMI state"),
        config_{ ctx_ /*todo revert to propagate dbus connection*/, fmt::format("atv320_i{}", slave_index) },
        ctrl_(connection, client, slave_index), dbus_iface_(ctrl_, connection, slave_index),
        reset_(ctx_, client, fmt::format("atv320.s{}.reset", slave_index), "Reset atv fault", [this](bool value) {
          auto timer = std::make_shared<asio::steady_timer>(ctx_);
          // A timer to reset the reset just in case
          allow_reset_ = value;
        }) {
    config_->observe([this](auto& new_value, auto& old_value) {
      logger_.warn(
          "Live motor configuration is discouraged. Large amounts of SDO traffic can delay and disrubt the ethercat cycle. "
          "Please consider turning of the ethercat master and editing the files directly if commisioning the device");
      if (new_value.nominal_motor_power != old_value.nominal_motor_power) {
        asio::post(ctx_, [this, new_value] { sdo_write(new_value.nominal_motor_power); });
      }
      if (new_value.nominal_motor_voltage != old_value.nominal_motor_voltage) {
        asio::post(ctx_, [this, new_value] { sdo_write(new_value.nominal_motor_voltage); });
      }
      if (new_value.nominal_motor_current != old_value.nominal_motor_current) {
        asio::post(ctx_, [this, new_value] { sdo_write(new_value.nominal_motor_current); });
      }
      if (new_value.nominal_motor_frequency != old_value.nominal_motor_frequency) {
        asio::post(ctx_, [this, new_value] { sdo_write(new_value.nominal_motor_frequency); });
      }
      if (new_value.nominal_motor_speed != old_value.nominal_motor_speed) {
        asio::post(ctx_, [this, new_value] { sdo_write(new_value.nominal_motor_speed); });
      }
      if (new_value.max_frequency != old_value.max_frequency) {
        asio::post(ctx_, [this, new_value] { sdo_write(new_value.max_frequency); });
      }
      if (new_value.motor_thermal_current != old_value.motor_thermal_current) {
        asio::post(ctx_, [this, new_value] { sdo_write(new_value.motor_thermal_current); });
      }
      if (new_value.high_speed != old_value.high_speed) {
        asio::post(ctx_, [this, new_value] { sdo_write(new_value.high_speed); });
      }
      if (new_value.low_speed != old_value.low_speed) {
        asio::post(ctx_, [this, new_value] { sdo_write(new_value.low_speed); });
      }
      if (new_value.motor_1_cos_phi != old_value.motor_1_cos_phi) {
        asio::post(ctx_, [this, new_value] { sdo_write(new_value.motor_1_cos_phi); });
      }
      if (new_value.fast_stop_ramp_divider != old_value.fast_stop_ramp_divider) {
        asio::post(ctx_, [this, new_value] { sdo_write(new_value.fast_stop_ramp_divider); });
      }
      if (new_value.async_motor_leakage_inductance != old_value.async_motor_leakage_inductance) {
        asio::post(ctx_, [this, new_value] { sdo_write(new_value.async_motor_leakage_inductance); });
      }
      if (new_value.async_motor_stator_resistance != old_value.async_motor_stator_resistance) {
        asio::post(ctx_, [this, new_value] { sdo_write(config_->value().async_motor_stator_resistance); });
      }
      if (new_value.rotor_time_constant != old_value.rotor_time_constant) {
        asio::post(ctx_, [this, new_value] { sdo_write(config_->value().rotor_time_constant); });
      }
    });

    config_->value().default_speedratio.observe(
        [this](speedratio_t new_v, auto) { dbus_iface_.set_configured_speedratio(new_v); });

    // Apply the default speedratio to both means of controlling the motor
    dbus_iface_.set_configured_speedratio(config_->value().default_speedratio);
    ctrl_.set_motor_nominal_freq(config_->value().nominal_motor_frequency.value);
    reference_frequency_ = detail::percentage_to_deci_freq(config_->value().default_speedratio, config_->value().low_speed,
                                                           config_->value().high_speed);

    for (size_t i = 0; i < atv320_di_count; i++) {
      di_transmitters_.emplace_back(tfc::ipc::bool_signal(connection->get_io_context(), client,
                                                          fmt::format("atv320.s{}.in{}", slave_index, i), "Digital Input"));
    }
  }

  // Update signals of the current status of the drive
  void transmit_status(const input_t& input) {
    std::bitset<atv320_di_count> const value(input.digital_inputs);
    for (size_t i = 0; i < atv320_di_count; i++) {
      last_bool_values_.set(
          i, details::async_send_if_new(di_transmitters_[i], last_bool_values_.test(i), value.test(i), logger_));
    }
    last_hmis_ = static_cast<hmis_e>(details::async_send_if_new(hmis_transmitter_, static_cast<uint16_t>(last_hmis_),
                                                                static_cast<uint16_t>(input.drive_state), logger_));
    double frequency = static_cast<double>(input.frequency.numerical_value_is_an_implementation_detail_) / 10.0;
    last_frequency_ = details::async_send_if_new(frequency_transmit_, last_frequency_, frequency, logger_);

    double current = static_cast<double>(input.current) / 10.0;
    last_current_ = details::async_send_if_new(current_transmit_, last_current_, current, logger_);

    last_error_ =
        details::async_send_if_new(last_error_transmit_, last_error_, static_cast<std::uint64_t>(last_errors_[0]), logger_);
  }

  static constexpr auto errors_to_auto_reset = std::array{ lft_e::no_fault, lft_e::cnf };

  auto process_data(std::span<std::byte> input, std::span<std::byte> output) noexcept -> void final {
    // All registers in the ATV320 ar uint16, create a pointer to this memory
    // With the same size
    // these sizes are in bytes not uint16_t
    if ((input.size() != sizeof(input_t) || output.size() != sizeof(output_t)) && !no_data_) {
      // First pdo cycle with no data
      no_data_ = true;
      input_t status{ .status_word = cia_402::status_word{ .state_fault = true },
                      .frequency = 0 * dHz,
                      .current = 0,
                      .digital_inputs = 0,
                      .last_error = lft_e::cnf,
                      .drive_state = hmis_e::fault };
      transmit_status(status);
      dbus_iface_.update_status(status);
      return;
    }
    if (input.size() != sizeof(input_t) || output.size() != sizeof(output_t)) {
      // All following pdo cycles with no data
      return;
    }

    // Reset no data bit
    no_data_ = false;
    const input_t* in = std::launder(reinterpret_cast<input_t*>(input.data()));
    output_t* out = std::launder(reinterpret_cast<output_t*>(output.data()));

    // Check if the drive is in error state
    auto state = in->status_word.parse_state();
    bool drive_in_fault_state = cia_402::states_e::fault == state;
    if (drive_in_fault_state && last_errors_[0] != in->last_error && in->last_error != lft_e::no_fault) {
      // We have a new fault on the drive
      std::shift_right(last_errors_.begin(), last_errors_.end(), 1);
      last_errors_[0] = in->last_error;
      bool auto_reset =
          std::find(errors_to_auto_reset.begin(), errors_to_auto_reset.end(), in->last_error) != errors_to_auto_reset.end();
      logger_.error("New fault detected {}, will try to auto reset: {}", in->last_error, auto_reset);
    } else if (drive_in_fault_state && in->last_error == lft_e::no_fault) {
      logger_.warn("ATV reports fault state but last fault is not set");
    } else if (in->last_error != last_errors_[0]) {
      logger_.warn("Atv not in fault state but reporting fault: {}, state: {}", in->last_error, state);
      std::shift_right(last_errors_.begin(), last_errors_.end(), 1);
      last_errors_[0] = in->last_error;
    }

    transmit_status(*in);
    dbus_iface_.update_status(*in);

    bool auto_reset_allowed =
        std::find(errors_to_auto_reset.begin(), errors_to_auto_reset.end(), in->last_error) != errors_to_auto_reset.end() ||
        allow_reset_;

    if (!dbus_iface_.has_peer()) {
      // Quick stop if frequncy set to 0
      auto action = cia_402::transition_action::none;
      if (ipc_running_)
        action = cia_402::transition_action::run;
      if (reference_frequency_.value == 0 * dHz)
        action = cia_402::transition_action::quick_stop;
      out->acc = config_->value().acceleration.value;
      out->dec = config_->value().deceleration.value;
      out->control = cia_402::transition(state, action, auto_reset_allowed);
      out->frequency = reference_frequency_.value;
    } else {
      auto freq =
          detail::percentage_to_deci_freq(ctrl_.speed_ratio(), config_->value().low_speed, config_->value().high_speed);
      out->acc = ctrl_.acceleration(config_->value().acceleration.value);
      out->dec = ctrl_.deceleration(config_->value().deceleration.value);
      out->frequency = freq.value;
      out->control = ctrl_.ctrl(auto_reset_allowed);

      // Set running to false. Will need to be set high before the motor starts on ipc
      // after dbus disconnect
      ipc_running_ = false;
    }

    // Write the reset allowed bit down after it has been up for 5 seconds.
    if (allow_reset_ && reset_timer_.expiry() < std::chrono::steady_clock::now()) {
      reset_timer_.expires_after(std::chrono::seconds(5));
      reset_timer_.async_wait([this](const std::error_code& err) {
        if (err)
          return;
        allow_reset_ = false;
      });
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
    sdo_write<uint32_t>(ecx::tx_pdo_mapping<0x05>, 0x20291610);  // LFT  - Last error occured
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
    sdo_write<uint32_t>(ecx::rx_pdo_mapping<0x04>, 0x203C0210);  // ACC - Acceleration
    sdo_write<uint32_t>(ecx::rx_pdo_mapping<0x05>, 0x203C0310);  // DEC - Deceleration

    // Set tx size
    sdo_write<uint8_t>(ecx::rx_pdo_mapping<0x00>, 5);

    // // Assign pdo's to mappings
    sdo_write<uint16_t>(ecx::rx_pdo_assign<0x01>, ecx::rx_pdo_mapping<>.first);
    sdo_write<uint8_t>(ecx::rx_pdo_assign<0x00>, 1);

    sdo_write<uint16_t>(ecx::tx_pdo_assign<0x01>, ecx::tx_pdo_mapping<>.first);
    sdo_write<uint8_t>(ecx::tx_pdo_assign<0x00>, 1);

    sdo_write(configuration_reference_frequency_1_FR1{ .value = psa_e::reference_frequency_via_com_module });
    // Clear internal ATV Functionality for outputs and inputs
    sdo_write(assignment_R1{ .value = psl_e::not_assigned });
    sdo_write(assignment_AQ1{ .value = psa_e::not_configured });

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
    sdo_write(config_->value().fast_stop_ramp_divider);
    sdo_write(config_->value().async_motor_leakage_inductance);
    sdo_write(config_->value().async_motor_stator_resistance);
    sdo_write(config_->value().rotor_time_constant);
    return 1;
  }

private:
  asio::io_context& ctx_;
  std::bitset<atv320_di_count> last_bool_values_;
  std::vector<tfc::ipc::bool_signal> di_transmitters_;
  bool ipc_running_{};
  ipc::bool_slot run_;
  detail::speed reference_frequency_{ .value = 0 * dHz };
  ipc::double_slot ratio_;
  double last_frequency_{};
  double last_current_{};
  std::uint64_t last_error_{};
  ipc::double_signal frequency_transmit_;
  ipc::double_signal current_transmit_;
  ipc::uint_signal last_error_transmit_;
  hmis_e last_hmis_{};
  ipc::uint_signal hmis_transmitter_;
  config_t config_;
  controller<manager_client_t> ctrl_;
  dbus_iface dbus_iface_;
  ipc::bool_slot reset_;
  std::array<lft_e, 10> last_errors_{};
  asio::steady_timer reset_timer_{ ctx_ };
  bool no_data_{ false };
  bool allow_reset_{ false };
};
}  // namespace tfc::ec::devices::schneider::atv320
