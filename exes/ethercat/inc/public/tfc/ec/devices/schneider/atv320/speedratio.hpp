#pragma once
#include <mp-units/math.h>
#include <mp-units/systems/si/si.h>

namespace tfc::ec::devices::schneider::atv320::detail {

constexpr auto percentage_to_deci_freq(mp_units::quantity<mp_units::percent, double> percentage,
                                       [[maybe_unused]] low_speed_LSP min_freq,
                                       [[maybe_unused]] high_speed_HSP max_freq) noexcept -> decifrequency_signed {
  if (mp_units::abs(percentage) < 1 * mp_units::percent) {
    return 0 * dHz;
  }
  mp_units::Quantity auto mapped{ ec::util::map(mp_units::abs(percentage), (1.0 * mp_units::percent),
                                                (100.0 * mp_units::percent), min_freq.value, max_freq.value) };
  if (percentage <= -1 * mp_units::percent) {
    mapped = -1 * mapped;
  }
  return mapped;
}
// gcc only supports constexpr std::abs and there is no feature flag
#ifndef __clang__
// stop test
static_assert(percentage_to_deci_freq(0 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) == 0 * dHz);
// min test forward
static_assert(percentage_to_deci_freq(1 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) == 200 * dHz);
// max test forward
static_assert(percentage_to_deci_freq(100 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) == 500 * dHz);
// 50% test forward
static_assert(percentage_to_deci_freq(50 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) == 348 * dHz);
// max test reverse
static_assert(percentage_to_deci_freq(-100 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) == -500 * dHz);
// min test reverse
static_assert(percentage_to_deci_freq(-1 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) == -200 * dHz);
// 50% test reverse
static_assert(percentage_to_deci_freq(-50 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) == -348 * dHz);
// outside bounds reverse
static_assert(percentage_to_deci_freq(-10000 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) == -500 * dHz);
// outside bounds forward
static_assert(percentage_to_deci_freq(10000 * mp_units::percent,
                                      low_speed_LSP{ .value = 200 * dHz },
                                      high_speed_HSP{ .value = 500 * dHz }) == 500 * dHz);
#endif
}  // namespace tfc::ec::devices::schneider::atv320::detail
