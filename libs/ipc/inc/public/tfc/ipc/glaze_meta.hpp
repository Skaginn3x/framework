#pragma once

#include <glaze/core/common.hpp>

#include <tfc/ipc/enums.hpp>
#include <tfc/stx/basic_fixed_string.hpp>

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
    tfc::ipc::details::type_e_iterable[std::to_underlying(_json)], _json, "Json"
  ) };
  // clang-format on
};

template <>
struct meta<tfc::ipc::details::mass_error_e> {
  using enum tfc::ipc::details::mass_error_e;
  // clang-format off
  static auto constexpr value{ glz::enumerate(
    "no_error", no_error, "Not an error",
    "cell_fault", cell_fault, "Load cell fault",
    "module_fault", module_fault, "General weigher module fault",
    "power_failure", power_failure, "Supply voltage too low",
    "over_range", over_range, "Over max value, please take care of the load cell",
    "under_range", under_range, "Will this be used? tbd",
    "bad_connection", bad_connection, "Bad connection to module",
    "zero_error", zero_error, "Zero error, something is probably stuck on the weigher",
    "calibration_error", calibration_error, "Calibration error",
    "not_calibrated", not_calibrated, "Not calibrated",
    "unknown_error", unknown_error, "Unknown error"
    ) };
  // clang-format on
  static constexpr std::string_view name{ "ipc::mass_error_e" };
};

}  // namespace glz
