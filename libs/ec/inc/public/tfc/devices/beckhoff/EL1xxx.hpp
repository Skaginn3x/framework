#pragma once

#include <tfc/ipc.hpp>

namespace tfc::ec::devices::beckhoff {
template <size_t size, uint32_t pc>
class el100x : public base {
public:
  explicit el100x(boost::asio::io_context& ctx) : base(ctx) {}
  static constexpr uint32_t product_code = pc;
  static constexpr auto vendor_id = 0x2;

  void process_data(uint8_t* input, uint8_t*) noexcept final {
    if (input == nullptr) {
      return;
    }
    static_assert(size <= 8);
    std::bitset<size> const in_bits(*input);
    for (size_t i = 0; i < size; i++) {
      bool const value = in_bits.test(i);
      if (value != last_values_[i]) {
        transmitters_[i]->async_send(value, [](std::error_code error, size_t) {
          if (error) {
            printf("EL1100, error transmitting : %s\n", error.message().c_str());
          }
        });
      }
      last_values_[i] = value;
    }
  }
  auto setup(ecx_contextt*, uint16_t slave_index) -> int final {
    for (size_t i = 0; i < size; i++) {
      std::expected<std::shared_ptr<tfc::ipc::bool_send>, std::error_code> ptr =
          tfc::ipc::bool_send::create(ctx_, fmt::format("EL100{}.{}.in.{}", size, slave_index, i));
      if (!ptr) {
        printf("%s\n", ptr.error().message().c_str());
        std::terminate();
      }
      transmitters_.push_back(ptr.value());
    }
    return 1;
  }

private:
  std::array<bool, size> last_values_;
  std::vector<std::shared_ptr<tfc::ipc::bool_send>> transmitters_;
};

using el1008 = el100x<8, 0x3f03052>;
}  // namespace tfc::ec::devices::beckhoff