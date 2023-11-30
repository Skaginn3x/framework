#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <utility>

#include <glaze/core/common.hpp>

#include <tfc/stx/basic_fixed_string.hpp>

// Common enums, ipc/glaze_meta.hpp provides glaze specific conversions

namespace tfc::ipc::details {

enum struct direction_e : std::uint8_t {
  unknown = 0,
  signal = 1,
  slot = 2,
};

/// \brief Finite set of types which can be sent over this protocol
/// \note _json is sent as packet<std::string, _json>
enum struct type_e : std::uint8_t {
  unknown = 0,
  _bool = 1,      // NOLINT
  _int64_t = 2,   // NOLINT
  _uint64_t = 3,  // NOLINT
  _double_t = 4,  // NOLINT
  _string = 5,    // NOLINT
  _json = 6,      // NOLINT
  _mass = 7,    // NOLINT
  // TODO: Add
  //  Standard units
  //  _duration = 7,
  //  _timepoint = 8,
  //  _velocity = 9,
  //  _temperature = 10,
  //  _humitidy = 11,

};

enum struct mass_error_e : std::uint8_t {
  no_error = 0,
  cell_fault,
  module_fault,
  power_failure, // Supply voltage too low
  over_range, // Over max value
  under_range, // Under min value, probably not used
  bad_connection,
  zero_error,
  calibration_error,
  not_calibrated,
  unknown_error,
};

static constexpr std::array<std::string_view, 8> type_e_iterable{ "unknown", "bool",   "int64_t", "uint64_t",
                                                                  "double",  "string", "json", "mass" };

auto constexpr enum_name(type_e type) -> std::string_view {
  return type_e_iterable[std::to_underlying(type)];
}

auto constexpr enum_cast(std::string_view name) -> type_e {
  for (std::size_t idx = type_e_iterable.size() - 1; idx > 0; idx--) {
    if (name.contains(type_e_iterable[idx])) {
      return static_cast<type_e>(idx);
    }
  }
  return type_e::unknown;
}

static_assert(enum_cast("blabb") == type_e::unknown);
static_assert(enum_cast("unknown") == type_e::unknown);
static_assert(enum_cast("bool") == type_e::_bool);
static_assert(enum_cast("int64_t") == type_e::_int64_t);
static_assert(enum_cast("uint64_t") == type_e::_uint64_t);
static_assert(enum_cast("double") == type_e::_double_t);
static_assert(enum_cast("string") == type_e::_string);
static_assert(enum_cast("json") == type_e::_json);

static_assert(enum_name(type_e::unknown) == "unknown");
static_assert(enum_name(type_e::_bool) == "bool");
static_assert(enum_name(type_e::_int64_t) == "int64_t");
static_assert(enum_name(type_e::_uint64_t) == "uint64_t");
static_assert(enum_name(type_e::_double_t) == "double");
static_assert(enum_name(type_e::_string) == "string");
static_assert(enum_name(type_e::_json) == "json");

}  // namespace tfc::ipc::details


namespace tfc::ipc {

inline stx::basic_fixed_string constexpr signal_tag{ "signal" };
inline stx::basic_fixed_string constexpr slot_tag{ "slot" };

}  // namespace tfc::ipc

namespace glz {

template <>
struct meta<tfc::ipc::details::direction_e> {
  using enum tfc::ipc::details::direction_e;
  // clang-format off
  static auto constexpr value{ glz::enumerate("unknown", unknown, "Unspecified direction",
                                              tfc::ipc::signal_tag.data_, signal, "Owner of information being sent/published",
                                              tfc::ipc::slot_tag.data_, slot, "Receiver of information, or subscriber")
  };
  // clang-format on
  static std::string_view constexpr name{ "direction_e" };
};

template <>
struct meta<tfc::ipc::details::type_e> {
  using enum tfc::ipc::details::type_e;
  // clang-format off
  static auto constexpr value{ glz::enumerate(
    tfc::ipc::details::type_e_iterable[std::to_underlying(unknown)], unknown, "Unspecified type",
    tfc::ipc::details::type_e_iterable[std::to_underlying(_bool)], _bool, "Boolean",
    tfc::ipc::details::type_e_iterable[std::to_underlying(_int64_t)], _int64_t, "Signed 64bit integer",
    tfc::ipc::details::type_e_iterable[std::to_underlying(_uint64_t)], _uint64_t, "Unsigned 64bit integer",
    tfc::ipc::details::type_e_iterable[std::to_underlying(_double_t)], _double_t, "Double",
    tfc::ipc::details::type_e_iterable[std::to_underlying(_string)], _string, "String",
    tfc::ipc::details::type_e_iterable[std::to_underlying(_json)], _json, "Json",
    tfc::ipc::details::type_e_iterable[std::to_underlying(_mass)], _mass, "Mass"
  ) };
  // clang-format on
};

}  // namespace glz
