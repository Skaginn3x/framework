#include <boost/ut.hpp>
#include <string_view>
#include "tfc/logger.hpp"
#include "tfc/progbase.hpp"

using std::string_view_literals::operator""sv;

auto main(int argc, char** argv) -> int {
  using boost::ut::operator""_test;
  using boost::ut::expect;

  tfc::base::init(argc, argv);

  "example logging"_test = [] {
    tfc::logger::logger foo("key");

    foo.log<tfc::logger::lvl_e::info>(""sv);
    foo.trace(""sv);
    foo.trace("Some arguments {}: {}", 1, 2);
    foo.debug("Some arguments {}: {}, {}", 1, 2, 3);
    foo.info("Some arguments {}: {}, {}, {}", 1, 2, 3, 4);
    foo.warn("Some arguments {}: {}, {}, {}, {}", 1, 2, 3, 4, 5);
    foo.error("Some arguments {}: {}, {}, {}, {}, {}", 1, 2, 3, 4, 5, 6);
    foo.critical("Some arguments {}: {}, {}, {}, {}, {}, {}", 1, 2, 3, 4, 5, 6, 7);
    foo.log<tfc::logger::lvl_e::info>("Some arguments {}: {}, {}, {}, {}, {}, {}, {}", 1, 2, 3, 4, 5, 6, 7, 8);
    foo.log<tfc::logger::lvl_e::info>("Some arguments {}: {}, {}, {}, {}, {}, {}, {}, {}", 1, 2, 3, 4, 5, 6, 7, 8, 9);
    foo.log<tfc::logger::lvl_e::info>("Some arguments {}: {}, {}, {}, {}, {}, {}, {}, {}, {}", 1, 2, 3, 4, 5, 6, 7, 8, 9,
                                      10);

    expect(true);
  };
}
