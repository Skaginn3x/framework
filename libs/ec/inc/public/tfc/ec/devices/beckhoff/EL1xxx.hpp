#pragma once

#include <tfc/ipc.hpp>

namespace tfc::ec::devices::beckhoff {
template <size_t size, uint32_t pc>
class el100x : public base {
public:
  explicit el100x(boost::asio::io_context& ctx, uint16_t const slave_index) : base(slave_index) {
    for (size_t i = 0; i < size; i++) {
      std::expected<std::shared_ptr<tfc::ipc::bool_send>, std::error_code> ptr =
          tfc::ipc::bool_send::create(ctx, fmt::format("EL100{}.{}.in.{}", size, slave_index, i));
      if (!ptr) {
        logger_.error("Ethercat, EL110x error: {}", ptr.error().message());
        std::terminate();
      }
      transmitters_.push_back(ptr.value());
    }
  }
  static constexpr uint32_t product_code = pc;
  static constexpr uint32_t vendor_id = 0x2;

  void process_data(std::span<std::byte> input, std::span<std::byte>) noexcept final {
    static_assert(size <= 8);
    std::bitset<size> const in_bits(static_cast<uint8_t>(input[0]));
    for (size_t i = 0; i < size; i++) {
      bool const value = in_bits.test(i);
      if (value != last_values_[i]) {
        transmitters_[i]->async_send(value, [this](std::error_code error, size_t) {
          if (error) {
            logger_.error("Ethercat EL110x, error transmitting : {}", error.message());
          }
        });
      }
      last_values_[i] = value;
    }
  }

private:
  std::array<bool, size> last_values_;
  std::vector<std::shared_ptr<tfc::ipc::bool_send>> transmitters_;
};

using el1008 = el100x<8, 0x3f03052>;
}  // namespace tfc::ec::devices::beckhoff
