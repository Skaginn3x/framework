#pragma once


namespace tfc::unit {

struct millimetre_per_second : ::units::derived_scaled_unit<millimetre_per_second,
                                                            ::units::isq::si::dim_speed,
                                                            ::units::isq::si::millimetre,
                                                            ::units::isq::si::second> {};
struct decihertz : units::prefixed_unit<decihertz, ::units::isq::si::deci, ::units::isq::si::hertz> {};
struct hectowatt : units::prefixed_unit<hectowatt, units::isq::si::hecto, units::isq::si::watt> {};
}  // namespace tfc::unit

// todo celsius
