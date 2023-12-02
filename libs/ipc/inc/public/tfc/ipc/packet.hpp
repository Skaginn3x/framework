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

namespace tfc::ipc::details {

/// \brief Enum specifying protocol version
/// This can be changed in the future to retain backwards compatibility and still
/// be able to change the protocol structure
enum struct version_e : std::uint8_t { unknown, v0 };

template <type_e type_enum>
struct header_t {
  static constexpr auto type_v{ type_enum };
  version_e version{ version_e::v0 };
  type_e type{ type_v };
  std::size_t value_size{};  // populated in deserialize
  // Todo crc
  static constexpr auto size() -> std::size_t { return sizeof(version) + sizeof(type) + sizeof(value_size); }
  static void serialize(header_t& header, auto&& buffer) {
    std::copy_n(reinterpret_cast<std::byte*>(&header.version), sizeof(version), std::back_inserter(buffer));
    std::copy_n(reinterpret_cast<std::byte*>(&header.type), sizeof(type), std::back_inserter(buffer));
    std::copy_n(reinterpret_cast<std::byte*>(&header.value_size), sizeof(value_size), std::back_inserter(buffer));
  }
  static auto deserialize(header_t& result, auto&& buffer_iter) -> std::error_code {
    std::copy_n(buffer_iter, sizeof(version), reinterpret_cast<std::byte*>(&result.version));
    buffer_iter += sizeof(version);
    std::copy_n(buffer_iter, sizeof(type), reinterpret_cast<std::byte*>(&result.type));
    buffer_iter += sizeof(type);
    std::copy_n(buffer_iter, sizeof(value_size), reinterpret_cast<std::byte*>(&result.value_size));
    buffer_iter += sizeof(value_size);

    if (result.type != type_v) {
      return std::make_error_code(std::errc::wrong_protocol_type);
    }
    if (result.version != version_e::v0) {
      return std::make_error_code(std::errc::wrong_protocol_type);
      // TODO: explicit version error
    }
    return {};
  }
};
static_assert(header_t<type_e::unknown>::size() == 10);

/// \brief packet struct to de/serialize data to socket
template <typename value_type, type_e type_enum>
struct packet {
  using value_t = value_type;
  static constexpr auto type_v{ type_enum };

  header_t<type_enum> header{};
  value_t value{};

  // value size is populated
  static auto serialize(value_t const& value, std::vector<std::byte>& buffer) -> std::error_code {
    header_t<type_enum> my_header{};

    if constexpr (std::is_fundamental_v<value_t>) {
      my_header.value_size = sizeof(value_t);
    } else {
      static_assert(std::is_member_function_pointer_v<decltype(&value_t::size)>, "Serialize for value type not supported");
      static_assert(std::is_same_v<decltype(value_t().size()), std::size_t>);
      my_header.value_size = value.size();
    }

    const std::size_t buffer_size{ header_t<type_enum>::size() + my_header.value_size };
    buffer.reserve(buffer_size);
    header_t<type_enum>::serialize(my_header, buffer);

    if constexpr (std::is_fundamental_v<value_t>) {
      std::copy_n(reinterpret_cast<std::byte const*>(&value), my_header.value_size, std::back_inserter(buffer));
    } else {
      // has member function data
      static_assert(std::is_pointer_v<decltype(value.data())>);
      // Todo why copy the data instead of creating a view into the data?
      // we should use std::span
      std::copy_n(reinterpret_cast<std::byte const*>(value.data()), my_header.value_size, std::back_inserter(buffer));
    }

    if (buffer.size() != buffer_size) {
      return std::make_error_code(std::errc::message_size);
    }
    return {};
  }

  static constexpr auto deserialize(std::ranges::view auto&& buffer) -> std::expected<value_t, std::error_code> {
    if (buffer.size() < header_t<type_enum>::size()) {
      return std::unexpected(std::make_error_code(std::errc::message_size));
    }

    packet<value_t, type_v> result{};
    auto buffer_iter{ std::begin(buffer) };
    header_t<type_enum>::deserialize(result.header, buffer_iter);

    // todo partial buffer?
    if (buffer.size() != header_t<type_enum>::size() + result.header.value_size) {
      return std::unexpected(std::make_error_code(std::errc::message_size));
    }

    if constexpr (std::is_fundamental_v<value_t>) {
      static_assert(sizeof(value_t) <= 8);
      std::copy_n(buffer_iter, result.header.value_size, reinterpret_cast<std::byte*>(&result.value));
    } else {
      // has member function data
      result.value.resize(result.header.value_size);
      std::copy_n(buffer_iter, result.header.value_size, reinterpret_cast<std::byte*>(result.value.data()));
    }
    return std::move(result.value);
  }
};

template <typename value_t, type_e type_e_v>
auto operator==(const packet<value_t, type_e_v>& lhs, const packet<value_t, type_e_v>& rhs) noexcept -> bool {
  return lhs.version == rhs.version && lhs.type == rhs.type && lhs.value_size == rhs.value_size && lhs.value == rhs.value;
}

}  // namespace tfc::ipc::details
