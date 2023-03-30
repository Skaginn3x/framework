#pragma once

#include <bitset>
#include <cstdio>

#include "ethercat.h"
#include "tfc/devices/base.hpp"

#include "tfc/ipc.hpp"

namespace tfc::ec::devices::abt {
class easyecat : public base {
public:
  explicit easyecat(boost::asio::io_context& ctx) : base(ctx) {}
  static constexpr auto vendor_id = 0x79a;
  static constexpr auto product_code = 0xdefede;
  auto process_data(uint8_t* input, uint8_t* output) noexcept -> void final {
    if (input == nullptr) {
      return;
    }
    std::bitset<4> const in_bits(input[6]);
    for (size_t i = 0; i < 4; i++) {
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
    for (size_t i = 0; i < 2; i++) {
      uint8_t const value = input[i];
      if (value != last_analog_value_[i]) {
        analog_transmitters_[i]->async_send(value, [](std::error_code error, size_t) {
          if (error) {
            printf("easyecat, error transmitting : %s\n", error.message().c_str());
          }
        });
      }
      last_analog_value_[i] = value;
    }
    *output = output_states_.to_ulong() & 0x0f;
  }
  auto setup(ecx_contextt*, uint16_t slave_index) -> int final {
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
          std::bind(&easyecat::set_output, this, i, std::placeholders::_1));
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
    return 1;
  }

private:
  auto set_output(size_t position, bool value) -> void { output_states_.set(position, value); }
  std::bitset<4> output_states_;
  std::array<bool, 4> last_bool_value_;
  std::array<uint8_t, 2> last_analog_value_;
  std::vector<std::shared_ptr<tfc::ipc::bool_send>> bool_transmitters_;
  std::vector<std::shared_ptr<tfc::ipc::uint_send>> analog_transmitters_;
  std::vector<std::shared_ptr<tfc::ipc::bool_recv_cb>> bool_receivers_;
};

}  // namespace tfc::ec::devices::abt