#pragma once

#include <units/isq/si/length.h>
#include <units/isq/si/speed.h>
#include <units/isq/si/time.h>

namespace tfc::unit {

struct millimetre_per_second : ::units::derived_scaled_unit<millimetre_per_second,
                                                            ::units::isq::si::dim_speed,
                                                            ::units::isq::si::millimetre,
                                                            ::units::isq::si::second> {};

}  // namespace tfc::unit
