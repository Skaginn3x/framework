#pragma once

#include <glaze/core/common.hpp>

#include <tfc/ipc.hpp>
#include <tfc/ipc/packet.hpp>
#include <tfc/stx/basic_fixed_string.hpp>

namespace tfc::ipc {

inline stx::basic_fixed_string constexpr signal_tag{ "signal" };
inline stx::basic_fixed_string constexpr slot_tag{ "slot" };

}  // namespace tfc::ipc

namespace glz {

template <>
struct meta<tfc::ipc::direction_e> {
  using enum tfc::ipc::direction_e;
  // clang-format off
  static auto constexpr value{ glz::enumerate("unknown", unknown, "Unspecified direction",
                                              tfc::ipc::signal_tag.data_, signal, "Owner of information being sent/published",
                                              tfc::ipc::slot_tag.data_, slot, "Receiver of information, or subscriber")
  };
  // clang-format on
  static std::string_view constexpr name{ "direction_e" };
};

template <>
struct meta<tfc::ipc::type_e> {
  using enum tfc::ipc::type_e;
  // clang-format off
  static auto constexpr value{ glz::enumerate(
    tfc::ipc::type_e_iterable[std::to_underlying(unknown)], unknown, "Unspecified type",
    tfc::ipc::type_e_iterable[std::to_underlying(_bool)], _bool, "Boolean",
    tfc::ipc::type_e_iterable[std::to_underlying(_int64_t)], _int64_t, "Signed 64bit integer",
    tfc::ipc::type_e_iterable[std::to_underlying(_uint64_t)], _uint64_t, "Unsigned 64bit integer",
    tfc::ipc::type_e_iterable[std::to_underlying(_double_t)], _double_t, "Double",
    tfc::ipc::type_e_iterable[std::to_underlying(_string)], _string, "String",
    tfc::ipc::type_e_iterable[std::to_underlying(_json)], _json, "Json"
  ) };
  // clang-format on
};

}  // namespace glz
