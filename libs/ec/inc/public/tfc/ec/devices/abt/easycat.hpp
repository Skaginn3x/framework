#pragma once

#include <bitset>

#include <tfc/ipc.hpp>

#include <tfc/ec/devices/base.hpp>
#include <tfc/ec/soem_interface.hpp>

namespace tfc::ec::devices::abt {
class easyecat final : public base {
public:
  ~easyecat() final;
  explicit easyecat(boost::asio::io_context& ctx_, uint16_t const slave_index) : base(slave_index) {
    for (size_t i = 0; i < 4; i++) {
      std::expected<std::shared_ptr<tfc::ipc::bool_send>, std::error_code> ptr =
          tfc::ipc::bool_send::create(ctx_, fmt::format("easyecat.{}.bool.in.{}", slave_index, i));
      if (!ptr) {
        printf("%s\n", ptr.error().message().c_str());
        std::terminate();
      }
      bool_transmitters_.push_back(ptr.value());
      bool_receivers_.emplace_back(
          tfc::ipc::bool_recv_cb::create(ctx_, fmt::format("easyecat.{}.bool.out.{}", slave_index, i)));
      // TODO: Don't supply ipc signal name. IPC should do this by itself using confman?
      // As a test now, just connect it with the example signal that the test program creates
      bool_receivers_.back()->init(
          fmt::format("{}.{}.easyecat.1.bool.in.{}", tfc::base::get_exe_name(), tfc::base::get_proc_name(), i),
          [this, i](bool value) { output_states_.set(i, value); });
    }
    for (size_t i = 0; i < 2; i++) {
      std::expected<std::shared_ptr<tfc::ipc::uint_send>, std::error_code> ptr =
          tfc::ipc::uint_send::create(ctx_, fmt::format("easyecat.{}.uint.in.{}", slave_index, i));
      if (!ptr) {
        printf("%s\n", ptr.error().message().c_str());
        std::terminate();
      }
      analog_transmitters_.push_back(ptr.value());
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
  std::vector<tfc::ipc::bool_send_ptr> bool_transmitters_;
  std::vector<tfc::ipc::uint_send_ptr> analog_transmitters_;
  std::vector<tfc::ipc::bool_recv_cb_ptr> bool_receivers_;
};

}  // namespace tfc::ec::devices::abt
