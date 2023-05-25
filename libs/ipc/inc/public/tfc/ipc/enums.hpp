#pragma once

#include <cstdint>

#define EXPORT __attribute__((visibility("default")))

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

}  // namespace tfc::ipc::details
