#include <boost/program_options/options_description.hpp>
#include <boost/ut.hpp>
#include "tfc/logger.hpp"
#include "tfc/progbase.hpp"

auto main(int argc, char** argv) -> int {
  using boost::ut::operator""_test;
  using boost::ut::expect;

  // Initilize framework
  auto prog_desc{ tfc::base::default_description() };
  tfc::base::init(argc, argv, prog_desc);

  "example logging"_test = []() {
    tfc::logger::logger foo("key");

    foo.log<tfc::logger::lvl_e::info>("");
    foo.log<tfc::logger::lvl_e::info>("Some arguments {}: {}", 1, 2);
    foo.log<tfc::logger::lvl_e::info>("Some arguments {}: {}, {}", 1, 2, 3);

    boost::ut::expect(true);
  };
}
