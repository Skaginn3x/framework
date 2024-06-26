#include <type_traits>
#include <mp-units/systems/si.h>
#include <mp-units/systems/isq.h>

#include <boost/ut.hpp>
#include <tfc/progbase.hpp>
#include <tfc/motor.hpp>

using namespace mp_units::si::unit_symbols;
using namespace tfc::motor::impl;

int main(int argc, char** argv) {
  using boost::ut::operator""_test;
  using boost::ut::expect;

  tfc::base::init(argc, argv);
}

// Motor impl unit tests
static_assert(nominal_at_50Hz_to_frequency(500. * (mm / s), 1000. * (mm / s)) == 100. * Hz);
static_assert(nominal_at_50Hz_to_frequency(500. * (mm / s), 750. * (mm / s)) == 75. * Hz);
static_assert(nominal_at_50Hz_to_frequency(100. * (mm / s), 150. * (mm / s)) == 75. * Hz);
static_assert(nominal_at_50Hz_to_frequency(100. * (mm / s), 300. * (mm / s)) == 150. * Hz);
static_assert(nominal_at_50Hz_to_frequency(1000. * (mm / s), 500. * (mm / s)) == 25. * Hz);
