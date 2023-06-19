#pragma once

#include "settings.hpp"

namespace glz {

template <>
struct meta<tfc::ec::devices::schneider::lxm32m::settings::operation_mode_e> {
  using enum tfc::ec::devices::schneider::lxm32m::settings::operation_mode_e;
  // clang-format off
  static constexpr auto value{ glz::enumerate(
      "manual_or_autotuning", manual_or_autotuning,
      "motion_sequence", motion_sequence,
      "electronic_gear", electronic_gear,
      "jog", jog,
      "reserved", reserved,
      "profile_position", profile_position,
      "profile_velocity", profile_velocity,
      "profile_torque", profile_torque,
      "homing", homing,
      "interpolated_position", interpolated_position,
      "cyclic_synchronous_position", cyclic_synchronous_position,
      "cyclic_synchronous_velocity", cyclic_synchronous_velocity,
      "cyclic_synchronous_torque", cyclic_synchronous_torque
      ) };
  // clang-format on
  static constexpr std::string_view name{ "operation_mode" };
};

}  // namespace glz
