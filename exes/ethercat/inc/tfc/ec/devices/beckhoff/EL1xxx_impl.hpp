#pragma once

#include <fmt/format.h>

#include <tfc/ec/devices/beckhoff/EL1xxx.hpp>
#include <tfc/ipc.hpp>

namespace tfc::ec::devices::beckhoff {

template <typename manager_client_type, size_t size, uint32_t pc, template <typename, typename> typename signal_t>
el100x<manager_client_type, size, pc, signal_t>::el100x(asio::io_context& ctx,
                                                        manager_client_type& client,
                                                        const uint16_t slave_index)
    : base(slave_index) {
  for (size_t i = 0; i < size; i++) {
    transmitters_.emplace_back(std::make_unique<bool_signal_t>(
        ctx, client, fmt::format("EL100{}.{}.in.{}", size, slave_index, i), "Digital input"));
  }
}

template <typename manager_client_type, size_t size, uint32_t pc, template <typename, typename> typename signal_t>
void el100x<manager_client_type, size, pc, signal_t>::process_data(std::span<std::byte> input,
                                                                   std::span<std::byte>) noexcept {
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
}  // namespace tfc::ec::devices::beckhoff
