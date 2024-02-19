#pragma once

#include <bitset>

#include <tfc/ec/devices/base.hpp>
#include <tfc/ec/soem_interface.hpp>
#include <tfc/ipc.hpp>

namespace tfc::ec::devices::abt {
template <typename manager_client_type>
class easyecat final : public base {
public:
  explicit easyecat(boost::asio::io_context& ctx_, manager_client_type& client, uint16_t const slave_index)
      : base(slave_index), servo_{ ctx_, client, fmt::format("easyecat{}.servo", slave_index), "Servo", [](auto) {} } {
    for (size_t i = 0; i < 4; i++) {
      bool_transmitters_.emplace_back(ctx_, client, fmt::format("easyecat{}.in{}", slave_index, i), "Digital input");
      bool_receivers_[i] =
          std::make_unique<ipc::bool_slot>(ctx_, client, fmt::format("easyecat{}.out{}", slave_index, i), "Digital output",
                                           [this, i](bool value) { output_states_.set(i, value); });
    }
    for (size_t i = 0; i < 2; i++) {
      analog_transmitters_.push_back(
          tfc::ipc::uint_signal(ctx_, client, fmt::format("easyecat{}.in{}", slave_index, i), "Analog input"));
    }
  }

  static constexpr uint32_t vendor_id = 0x79a;
  static constexpr uint32_t product_code = 0xdefede;

  static constexpr size_t di_count = 4;  // Number of digital inputs
  static constexpr size_t do_count = 4;  // Number of digital outputs
  static constexpr size_t ai_count = 2;  // Number of analog inputs

  auto process_data(std::span<std::byte> input, std::span<std::byte> output) noexcept -> void final {
    std::bitset<di_count> const in_bits(static_cast<uint8_t>(input[6]));
    for (size_t i = 0; i < di_count; i++) {
      bool const value = in_bits.test(i);
      if (!last_bool_value_[i].has_value() || value != last_bool_value_[i]) {
        bool_transmitters_[i].async_send(value, [this](std::error_code error, size_t) {
          if (error) {
            this->logger_.info("bool error transmitting: {}", error.message().c_str());
          }
        });
      }
      last_bool_value_[i] = value;
    }
    for (size_t i = 0; i < ai_count; i++) {
      auto const value = static_cast<uint8_t>(input[i]);
      if (!last_analog_value_[i].has_value() || value != last_analog_value_[i]) {
        analog_transmitters_[i].async_send(value, [this](std::error_code error, size_t) {
          if (error) {
            this->logger_.info("analog error transmitting: {}", error.message().c_str());
          }
        });
      }
      last_analog_value_[i] = value;
    }
    output[0] = static_cast<std::byte>(output_states_.to_ulong() & 0x0f);
    if (auto const servo_value = servo_.value(); servo_value.has_value()) {
      output[1] = static_cast<std::byte>(servo_value.value());
    } else {
      output[1] = static_cast<std::byte>(0);
    }
  }

private:
  std::bitset<do_count> output_states_;
  std::array<std::optional<bool>, di_count> last_bool_value_;
  std::array<std::optional<uint8_t>, ai_count> last_analog_value_;
  std::vector<ipc::bool_signal> bool_transmitters_;
  std::vector<ipc::uint_signal> analog_transmitters_;
  std::array<std::unique_ptr<ipc::bool_slot>, di_count> bool_receivers_;
  ipc::uint_slot servo_;
};

}  // namespace tfc::ec::devices::abt
