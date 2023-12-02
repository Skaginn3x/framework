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

}  // namespace glz
