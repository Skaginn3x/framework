#include <iostream>

#include <boost/ut.hpp>

#include <tfc/ec/devices/util.hpp>

auto main(int, char**) -> int {
  using boost::ut::operator""_test;
  using boost::ut::expect;

  "mapping function"_test = []() {
    expect(tfc::ec::util::map(10, 0, 10, 0, 20) == 20);
    expect(tfc::ec::util::map(500, 0, 1000, 0, 20) == 10);
  };
}
