#pragma once

#include <fmt/format.h>

#include <tfc/ec/devices/beckhoff/EL1xxx.hpp>
#include <tfc/ipc.hpp>

namespace tfc::ec::devices::beckhoff {

template <typename manager_client_type,
          size_t size,
          std::array<std::size_t, size> entries,
          uint32_t pc,
          stx::basic_fixed_string name,
          template <typename, typename>
          typename signal_t>
el1xxx<manager_client_type, size, entries, pc, name, signal_t>::el1xxx(asio::io_context& ctx,
                                                                       manager_client_type& client,
                                                                       const uint16_t slave_index)
    : base<el1xxx<manager_client_type, size, entries, pc, name, signal_t>>(slave_index) {
  for (size_t i = 0; i < size; i++) {
    transmitters_[i] = std::make_unique<bool_signal_t>(
        ctx, client, fmt::format("{}.s{}.in{}", name.view(), slave_index, entries[i]), "Digital input");
    /// todo description: skápur - tæki - íhlutur
  }
}

template <typename manager_client_type,
          size_t size,
          std::array<std::size_t, size> entries,
          uint32_t pc,
          stx::basic_fixed_string name,
          template <typename, typename>
          typename signal_t>
void el1xxx<manager_client_type, size, entries, pc, name, signal_t>::pdo_cycle(input_pdo const& input,
                                                                               std::span<std::uint8_t>) noexcept {
  // Loop bytes
  for (size_t idx = 0; idx < input.size(); idx++) {
    for (size_t bits = 0; bits < 8 && size - ((idx * 8) + bits) > 0; bits++) {
      auto const value = static_cast<bool>(input[idx] & (1 << bits));
      const size_t bit_index = idx * 8 + bits;
      if (!last_values_[bit_index].has_value() || value != last_values_[bit_index]) {
        transmitters_[bit_index]->async_send(value, [this](std::error_code error, size_t) {
          if (error) {
            this->logger_.error("Ethercat {}, error transmitting : {}", name.view(), error.message());
          }
        });
      }
      last_values_[bit_index] = value;
    }
  }
}
}  // namespace tfc::ec::devices::beckhoff
