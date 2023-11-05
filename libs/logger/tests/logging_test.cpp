#include <boost/ut.hpp>
#include "tfc/logger.hpp"

import std;
import tfc.base;

using std::string_view_literals::operator""sv;

auto main(int argc, char** argv) -> int {
  using boost::ut::operator""_test;
  using boost::ut::expect;

  // Initilize framework
  tfc::base::init(argc, argv);

  "example logging"_test = []() {
    tfc::logger::logger foo("key");

    foo.log<tfc::logger::lvl_e::info>(""sv);
    foo.log<tfc::logger::lvl_e::info>("Some arguments {}: {}", 1, 2);
    foo.log<tfc::logger::lvl_e::info>("Some arguments {}: {}, {}", 1, 2, 3);

    boost::ut::expect(true);
  };
}
