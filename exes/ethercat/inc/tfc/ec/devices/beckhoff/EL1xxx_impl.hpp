#pragma once

#include <fmt/format.h>

#include <tfc/ec/devices/beckhoff/EL1xxx.hpp>
#include <tfc/ipc.hpp>

namespace tfc::ec::devices::beckhoff {

template <typename manager_client_type, size_t size, uint32_t pc, template <typename, typename> typename signal_t>
el1xxx<manager_client_type, size, pc, signal_t>::el1xxx(asio::io_context& ctx,
                                                        manager_client_type& client,
                                                        const uint16_t slave_index)
    : base(slave_index) {
  for (size_t i = 0; i < size; i++) {
    transmitters_.emplace_back(std::make_unique<bool_signal_t>(
        ctx, client, fmt::format("EL100{}.slave{}.in{}", size, slave_index, i), "Digital input"));
    // todo description: skápur - tæki - íhlutur
  }
}

template <typename manager_client_type, size_t size, uint32_t pc, template <typename, typename> typename signal_t>
void el1xxx<manager_client_type, size, pc, signal_t>::process_data(std::span<std::byte> input,
                                                                   std::span<std::byte>) noexcept {
  static_assert(size <= 16);
  auto* input_bits = reinterpret_cast<uint16_t*>(input.data());
  for (size_t i = 0; i < size; i++) {
    bool const value = *input_bits & (1 << i);
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
