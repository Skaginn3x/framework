#pragma once

#include <cstdint>

// Common enums, ipc/glaze_meta.hpp provides string conversions for the below enums

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

  // TODO: Add
  //  Standard units
  //  _duration = 7,
  //  _timepoint = 8,
  //  _velocity = 9,
  //  _temperature = 10,
  //  _humitidy = 11,

};

static constexpr std::array<std::string_view, 7> type_e_iterable{ "unknown", "bool",   "int64_t", "uint64_t",
                                                                  "double",  "string", "json" };

inline auto constexpr string_to_type(std::string_view name) -> type_e {
  for (std::size_t idx = 0; idx < type_e_iterable.size(); idx++) {
    if (type_e_iterable[idx] == name) {
      return static_cast<type_e>(idx);
    }
  }
  return type_e::unknown;
}

static_assert(string_to_type("blabb") == type_e::unknown);
static_assert(string_to_type("unknown") == type_e::unknown);
static_assert(string_to_type("bool") == type_e::_bool);
static_assert(string_to_type("int64_t") == type_e::_int64_t);
static_assert(string_to_type("uint64_t") == type_e::_uint64_t);
static_assert(string_to_type("double") == type_e::_double_t);
static_assert(string_to_type("string") == type_e::_string);
static_assert(string_to_type("json") == type_e::_json);

}  // namespace tfc::ipc::details
