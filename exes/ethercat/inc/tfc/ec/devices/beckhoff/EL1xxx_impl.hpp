#pragma once

#include <fmt/format.h>

#include <tfc/ec/devices/beckhoff/EL1xxx.hpp>
#include <tfc/ipc.hpp>

namespace tfc::ec::devices::beckhoff {

template <typename manager_client_type,
          size_t size,
          std::array<std::size_t, size> entries,
          uint32_t pc,
          tfc::stx::basic_fixed_string name,
          template <typename, typename>
          typename signal_t>
el1xxx<manager_client_type, size, entries, pc, name, signal_t>::el1xxx(asio::io_context& ctx,
                                                              manager_client_type& client,
                                                              const uint16_t slave_index)
    : base(slave_index) {
  for (size_t i = 0; i < size; i++) {
    transmitters_.emplace_back(std::make_unique<bool_signal_t>(
        ctx, client, fmt::format("{}.s{}.in{}", name.view(), slave_index, entries[i]), "Digital input"));
    /// todo description: skápur - tæki - íhlutur
  }
}

template <typename manager_client_type,
          size_t size,
          std::array<std::size_t, size> entries,
          uint32_t pc,
          tfc::stx::basic_fixed_string name,
          template <typename, typename>
          typename signal_t>
void el1xxx<manager_client_type, size, entries, pc, name, signal_t>::process_data(std::span<std::byte> input,
                                                                         std::span<std::byte>) noexcept {
  constexpr size_t minimum_byte_count = (size / 9) + 1;
  assert(input.size() == minimum_byte_count && "EL1XXX Size mismatch between process data and expected");

  // Loop bytes
  for (size_t byte = 0; byte < minimum_byte_count; byte++) {
    for (size_t bits = 0; bits < 8 && size - ((byte * 8) + bits) > 0; bits++) {
      auto const value = static_cast<bool>(static_cast<uint8_t>(input[byte]) & (1 << bits));
      const size_t bit_index = byte * 8 + bits;
      if (value != last_values_[bit_index]) {
        transmitters_[bit_index]->async_send(value, [this](std::error_code error, size_t) {
          if (error) {
            logger_.error("Ethercat {}, error transmitting : {}", name.view(), error.message());
          }
        });
      }
      last_values_[bit_index] = value;
    }
  }
}
}  // namespace tfc::ec::devices::beckhoff
