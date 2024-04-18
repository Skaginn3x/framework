#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <utility>

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
  _bool = 1,          // NOLINT
  _int64_t = 2,       // NOLINT
  _uint64_t = 3,      // NOLINT
  _double_t = 4,      // NOLINT
  _string = 5,        // NOLINT
  _json = 6,          // NOLINT
  _mass = 7,          // NOLINT
  _length = 8,        // NOLINT
  _pressure = 9,      // NOLINT
  _temperature = 10,  // NOLINT
  _voltage = 11,      // NOLINT
  _current = 12,      // NOLINT
  // TODO: Add
  //  Standard units
  //  _duration = 7,
  //  _timepoint = 8,
  //  _velocity = 9,
  //  _humitidy = 11,
};

static constexpr std::array<std::string_view, 13> type_e_iterable{ "unknown", "bool",     "int64_t",     "uint64_t",
                                                                   "double",  "string",   "json",        "mass",
                                                                   "length",  "pressure", "temperature", "voltage",
                                                                   "current" };

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

enum struct mass_error_e : std::uint8_t {
  no_error = 0,
  cell_fault,
  module_fault,
  power_failure,  // Supply voltage too low
  over_range,     // Over max value
  under_range,    // Under min value, used for example with ADC values less than 4 mA
  bad_connection,
  zero_error,
  calibration_error,
  not_calibrated,
  unknown_error,
};

enum struct sensor_error_e : std::uint8_t {
  no_error = 0,
  sensor_fault,
  over_range,
  under_range,
  bad_connection,
  unknown_error,
};

// Voltage
enum struct voltage_error_e : std::uint8_t {
  no_error = 0,
  over_range,
  under_range,
  short_circuit,
  unknown_error,
};

// Ampere
enum struct current_error_e : std::uint8_t {
  no_error = 0,
  over_range,
  under_range,
  short_circuit,
  unknown_error,
};

// todo use https://github.com/arturbac/simple_enum/
auto enum_name(mass_error_e) -> std::string_view;
auto enum_name(sensor_error_e) -> std::string_view;
auto enum_name(voltage_error_e) -> std::string_view;
auto enum_name(current_error_e) -> std::string_view;
// for fmt
inline auto format_as(mass_error_e err) -> std::string_view {
  return enum_name(err);
}
inline auto format_as(sensor_error_e err) -> std::string_view {
  return enum_name(err);
}
inline auto format_as(voltage_error_e err) -> std::string_view {
  return enum_name(err);
}
inline auto format_as(current_error_e err) -> std::string_view {
  return enum_name(err);
}

}  // namespace tfc::ipc::details
