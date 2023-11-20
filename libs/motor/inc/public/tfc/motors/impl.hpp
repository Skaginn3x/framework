#pragma once
namespace tfc::motor::impl {

using mp_units::QuantityOf;
using namespace mp_units::si::unit_symbols;
using namespace mp_units;

/**
 * @brief Retrieve a frequency to run a motor at, given it's nominal speed @ 50Hz and a linear relationship between the
 * motors frequency and output speed.
 * @param reference_speed Motor speed at 50Hz
 * @param target_speed target velocity
 * @return Frequency to maintain `target_speed`
 */
constexpr auto nominal_at_50Hz_to_frequency(QuantityOf<isq::velocity> auto reference_speed,
                                            QuantityOf<isq::velocity> auto target_speed) {
  return 50. * Hz * (target_speed / reference_speed);
}

}  // namespace tfc::motor::impl
