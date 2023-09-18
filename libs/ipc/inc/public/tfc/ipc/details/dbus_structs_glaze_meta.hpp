#pragma once

#include <glaze/core/common.hpp>

#include <tfc/ipc/details/dbus_structs.hpp>
#include <tfc/stx/glaze_meta.hpp>

template <>
struct glz::meta<tfc::ipc_ruler::signal> {
  using signal = tfc::ipc_ruler::signal;
  // clang-format off
  static constexpr auto value{ glz::object(
      "name", &signal::name,
      "type", &signal::type,
      "created_by", &signal::created_by,
      "created_at", &signal::created_at,
      "last_registered", &signal::last_registered,
      "description", &signal::description) };
  // clang-format on
  static constexpr std::string_view name{ "signal" };
};

template <>
struct glz::meta<tfc::ipc_ruler::slot> {
  using slot = tfc::ipc_ruler::slot;
  // clang-format off
  static constexpr auto value{ glz::object(
      "name", &slot::name,
      "type", &slot::type,
      "created_by", &slot::created_by,
      "created_at", &slot::created_at,
      "last_registered", &slot::last_registered,
      "last_modified", &slot::last_modified,
      "modified_by", &slot::modified_by,
      "connected_to", &slot::connected_to,
      "description", &slot::description) };
  // clang-format on
  static constexpr std::string_view name{ "slot" };
};
