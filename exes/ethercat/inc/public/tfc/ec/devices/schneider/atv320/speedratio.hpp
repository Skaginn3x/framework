#pragma once
#include <mp-units/systems/si/si.h>

namespace tfc::ec::devices::schneider::atv320::detail {
struct speed {
  decifrequency value{ 0 * dHz };
  bool reverse{ false };
  constexpr auto operator==(speed const& other) const noexcept -> bool = default;
};
constexpr auto percentage_to_deci_freq(mp_units::quantity<mp_units::percent, double> percentage,
                                       [[maybe_unused]] low_speed_LSP min_freq,
                                       [[maybe_unused]] high_speed_HSP max_freq) noexcept -> speed {
  if (mp_units::abs(percentage) < 1 * mp_units::percent) {
    return { .value = 0 * dHz, .reverse = false };
  }
  bool reverse = percentage.numerical_value_ <= -1;
  mp_units::Quantity auto mapped{ ec::util::map(mp_units::abs(percentage), (1.0 * mp_units::percent),
                                                (100.0 * mp_units::percent), min_freq.value, max_freq.value) };
  return { .value = mapped, .reverse = reverse };
}
// gcc only supports constexpr std::abs and there is no feature flag
#ifndef __clang__
// stop test
static_assert(percentage_to_deci_freq(0 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) == speed{ .value = 0 * dHz, .reverse = false });
// min test forward
static_assert(percentage_to_deci_freq(1 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) ==
              speed{ .value = 200 * dHz, .reverse = false });
// max test forward
static_assert(percentage_to_deci_freq(100 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) ==
              speed{ .value = 500 * dHz, .reverse = false });
// 50% test forward
static_assert(percentage_to_deci_freq(50 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) ==
              speed{ .value = 348 * dHz, .reverse = false });
// max test reverse
static_assert(percentage_to_deci_freq(-100 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) == speed{ .value = 500 * dHz, .reverse = true });
// min test reverse
static_assert(percentage_to_deci_freq(-1 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) == speed{ .value = 200 * dHz, .reverse = true });
// 50% test reverse
static_assert(percentage_to_deci_freq(-50 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) == speed{ .value = 348 * dHz, .reverse = true });
// outside bounds reverse
static_assert(percentage_to_deci_freq(-10000 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) == speed{ .value = 500 * dHz, .reverse = true });
// outside bounds forward
static_assert(percentage_to_deci_freq(10000 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) ==
              speed{ .value = 500 * dHz, .reverse = false });
#endif
}  // namespace tfc::ec::devices::schneider::atv320::detail
