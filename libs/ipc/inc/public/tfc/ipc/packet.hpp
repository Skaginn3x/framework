#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <ranges>
#include <type_traits>
#include <vector>

#include <tfc/ipc/enums.hpp>

#define EXPORT __attribute__((visibility("default")))

namespace tfc::ipc::details {

/// \brief Enum specifying protocol version
/// This can be changed in the future to retain backwards compatibility and still
/// be able to change the protocol structure
enum struct version_e : std::uint8_t { v0 };

enum struct packt_errors_e {
  inconsistent_buffer_size = 1,
};

/// \brief packet struct to de/serialize data to socket
template <typename value_type, type_e type_enum>
struct packet {
  using value_t = value_type;
  static constexpr auto type_v{ type_enum };

  version_e version{ version_e::v0 };
  type_e type{ type_v };
  std::size_t value_size{};  // populated in deserialize
  // Todo crc
  value_t value{};

  static constexpr auto header_size() -> std::size_t { return sizeof(version) + sizeof(type) + sizeof(value_size); }

  // value size is populated
  static auto serialize(packet& pack) -> std::expected<std::vector<std::byte>, std::error_code> {
    if constexpr (std::is_fundamental_v<value_t>) {
      pack.value_size = sizeof(value_t);
    } else {
      static_assert(std::is_member_function_pointer_v<decltype(&value_t::size)>, "Serialize for value type not supported");
      static_assert(std::is_same_v<decltype(value_t().size()), std::size_t>);
      pack.value_size = pack.value.size();
    }

    const std::size_t buffer_size{ header_size() + pack.value_size };
    std::vector<std::byte> buffer{};
    buffer.reserve(buffer_size);
    std::copy_n(reinterpret_cast<std::byte*>(&pack.version), sizeof(version), std::back_inserter(buffer));
    std::copy_n(reinterpret_cast<std::byte*>(&pack.type), sizeof(type), std::back_inserter(buffer));
    std::copy_n(reinterpret_cast<std::byte*>(&pack.value_size), sizeof(value_size), std::back_inserter(buffer));

    if constexpr (std::is_fundamental_v<value_t>) {
      std::copy_n(reinterpret_cast<std::byte*>(&pack.value), pack.value_size, std::back_inserter(buffer));
    } else {
      // has member function data
      static_assert(std::is_pointer_v<decltype(pack.value.data())>);
      // Todo why copy the data instead of creating a view into the data?
      // we should use std::span
      std::copy_n(reinterpret_cast<std::byte*>(pack.value.data()), pack.value_size, std::back_inserter(buffer));
    }

    if (buffer.size() != buffer_size) {
      // todo: Create custom error codes
      std::terminate();
      // return std::unexpected(packt_errors_e::inconsistent_buffer_size);
    }
    return buffer;
  }

  static constexpr auto deserialize(std::ranges::view auto buffer) -> packet<value_t, type_v> {
    packet<value_t, type_v> result{};
    auto buffer_iter{ std::begin(buffer) };
    std::copy_n(buffer_iter, sizeof(version), reinterpret_cast<std::byte*>(&result.version));
    buffer_iter += sizeof(version);
    std::copy_n(buffer_iter, sizeof(type), reinterpret_cast<std::byte*>(&result.type));
    buffer_iter += sizeof(type);
    std::copy_n(buffer_iter, sizeof(value_size), reinterpret_cast<std::byte*>(&result.value_size));
    buffer_iter += sizeof(value_size);

    if (result.type != type_v) {
      // TODO: Return error
    }
    if (result.version != version_e::v0) {
      // TODO: Return error
    }

    // todo partial buffer?
    if (buffer.size() != header_size() + result.value_size) {
      throw std::runtime_error("Inconsistent buffer size");
    }

    if constexpr (std::is_fundamental_v<value_t>) {
      static_assert(sizeof(value_t) <= 8);
      std::copy_n(buffer_iter, result.value_size, reinterpret_cast<std::byte*>(&result.value));
    } else {
      // has member function data
      result.value.resize(result.value_size);
      std::copy_n(buffer_iter, result.value_size, reinterpret_cast<std::byte*>(result.value.data()));
    }
    return result;
  }
};

template <typename value_t, type_e type_e_v>
auto operator==(const packet<value_t, type_e_v>& lhs, const packet<value_t, type_e_v>& rhs) noexcept -> bool {
  return lhs.version == rhs.version && lhs.type == rhs.type && lhs.value_size == rhs.value_size && lhs.value == rhs.value;
}

}  // namespace tfc::ipc::details
