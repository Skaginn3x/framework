#pragma once

#include <bitset>

#include <tfc/ec/devices/base.hpp>
#include <tfc/ec/soem_interface.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc_connector.hpp>

namespace tfc::ec::devices::abt {
class easyecat final : public base {
public:
  ~easyecat() final;

  explicit easyecat(boost::asio::io_context& ctx_, uint16_t const slave_index) : base(slave_index) {
    for (size_t i = 0; i < 4; i++) {
      // std::unique_ptr<tfc::ipc::bool_send_exposed> ptr =
      //     std::make_unique<tfc::ipc::bool_send_exposed>();
      bool_transmitters_.emplace_back(
          std::make_unique<tfc::ipc::bool_send_exposed>(ctx_, fmt::format("easyecat.{}.in.{}", slave_index, i)));
      bool_receivers_.emplace_back(
          std::make_unique<tfc::ipc::bool_recv_conf_cb>(ctx_, fmt::format("easyecat.{}.out.{}", slave_index, i),
                                                        [this, i](bool value) { output_states_.set(i, value); }));
    }
    for (size_t i = 0; i < 2; i++) {
      analog_transmitters_.push_back(
          std::make_unique<tfc::ipc::uint_send_exposed>(ctx_, fmt::format("easyecat.{}.in.{}", slave_index, i)));
    }
  }

  static constexpr uint32_t vendor_id = 0x79a;
  static constexpr uint32_t product_code = 0xdefede;

  // Number of digital inputs
  static constexpr size_t di_count = 4;
  // Number of digital outputs
  static constexpr size_t do_count = 4;
  // Number of analog inputs
  static constexpr size_t ai_count = 2;

  auto process_data(std::span<std::byte> input, std::span<std::byte> output) noexcept -> void final {
    std::bitset<di_count> const in_bits(static_cast<uint8_t>(input[6]));
    for (size_t i = 0; i < di_count; i++) {
      bool const value = in_bits.test(i);
      if (value != last_bool_value_[i]) {
        bool_transmitters_[i]->async_send(value, [](std::error_code error, size_t) {
          if (error) {
            printf("easyecat, error transmitting : %s\n", error.message().c_str());
          }
        });
      }
      last_bool_value_[i] = value;
    }
    for (size_t i = 0; i < ai_count; i++) {
      uint8_t const value = static_cast<uint8_t>(input[i]);
      if (value != last_analog_value_[i]) {
        analog_transmitters_[i]->async_send(value, [](std::error_code error, size_t) {
          if (error) {
            printf("easyecat, error transmitting : %s\n", error.message().c_str());
          }
        });
      }
      last_analog_value_[i] = value;
    }
    output[0] = static_cast<std::byte>(output_states_.to_ulong() & 0x0f);
  }

private:
  std::bitset<do_count> output_states_;
  std::array<bool, di_count> last_bool_value_;
  std::array<uint8_t, ai_count> last_analog_value_;
  std::vector<std::unique_ptr<tfc::ipc::bool_send_exposed>> bool_transmitters_;
  std::vector<std::unique_ptr<tfc::ipc::uint_send_exposed>> analog_transmitters_;
  std::vector<std::unique_ptr<tfc::ipc::bool_recv_conf_cb>> bool_receivers_;
};

}  // namespace tfc::ec::devices::abt
