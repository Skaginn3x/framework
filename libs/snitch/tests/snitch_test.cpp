#include <boost/ut.hpp>
#include <string_view>

#include <tfc/progbase.hpp>
#include <tfc/snitch.hpp>

using std::string_view_literals::operator""sv;

auto main(int argc, char** argv) -> int {
  using boost::ut::operator""_test;
  using boost::ut::expect;

  tfc::base::init(argc, argv);

  "snitches"_test = [] {
    tfc::snitch::info<"short desc", "long desc"> info(fmt::arg("key1", "value"), fmt::arg("key2", 1));
    fmt::print("{}", 1);
    // warn.on_acknowledgement([]{});

  };
}
