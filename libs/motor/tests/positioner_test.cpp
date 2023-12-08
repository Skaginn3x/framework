#include <type_traits>
#include <mp-units/systems/si/si.h>

#include <boost/ut.hpp>
#include <tfc/progbase.hpp>
#include <tfc/motors/positioner.hpp>

using namespace mp_units::si::unit_symbols;

int main(int argc, char** argv) {
  using boost::ut::operator""_test;
  using boost::ut::expect;

  tfc::base::init(argc, argv);

  return EXIT_SUCCESS;
}
