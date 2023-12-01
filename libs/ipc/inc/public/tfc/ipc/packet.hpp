#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <ranges>
#include <type_traits>
#include <vector>

#include <glaze/core/meta.hpp>
#include <glaze/core/common.hpp>

#include <tfc/ipc/enums.hpp>
#include <tfc/ipc/details/type_description.hpp>

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
  std::size_t payload_size{};  // populated in deserialize
};
static_assert(sizeof(header_t<type_e::unknown>) == 16);

/// \brief packet struct to de/serialize data to socket
template <concepts::is_supported_type value_type, type_e type_enum>
struct packet {
  using value_t = value_type;
  static constexpr auto type_v{ type_enum };

  header_t<type_enum> header{};
  value_t payload{};
};

template <typename value_t, type_e type_e_v>
auto operator==(const packet<value_t, type_e_v>& lhs, const packet<value_t, type_e_v>& rhs) noexcept -> bool {
  return lhs.version == rhs.version && lhs.type == rhs.type && lhs.value_size == rhs.value_size && lhs.value == rhs.value;
}

}  // namespace tfc::ipc::details

namespace glz {

using tfc::ipc::details::type_e;
using tfc::ipc::details::header_t;
using tfc::ipc::details::packet;
using tfc::ipc::details::concepts::is_supported_type;

template <type_e type_enum>
struct meta<header_t<type_enum>> {
  using self = header_t<type_enum>;
  static std::string_view constexpr name{ "ipc::header_t" };
  static auto constexpr value{ glz::object("version", &self::version, "Protocol version",
                                              "type", &self::type, "Type of the value",
                                              "payload_size", &self::payload_size, "Size of the value in bytes") };
};

template <is_supported_type value_t, type_e type_enum>
struct meta<packet<value_t, type_enum>> {
  using self = packet<value_t, type_enum>;
  static std::string_view constexpr name{ "ipc::packet" };
  static auto constexpr value{ glz::object("header", &self::header, "Header of the packet",
                                              "payload", &self::payload, "Value of the packet") };
};

}  // namespace glz

