#include <boost/program_options/options_description.hpp>
#include <boost/ut.hpp>
#include <string_view>
#include "tfc/logger.hpp"
#include "tfc/progbase.hpp"

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
