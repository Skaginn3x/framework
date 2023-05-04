#include "tfc/dbus_util.hpp"
#include <boost/program_options/options_description.hpp>
#include <boost/ut.hpp>
#include <string_view>
#include "tfc/logger.hpp"
#include "tfc/progbase.hpp"

using std::string_view_literals::operator""sv;

auto main(int argc, char** argv) -> int {
  using boost::ut::operator""_test;
  using boost::ut::expect;

  tfc::base::init(argc, argv);

  "dbus runtime exception test"_test = []() {
    tfc::dbus::exception::runtime t("desc");

    boost::ut::expect(strcmp(t.what(), "desc") == 0);
    boost::ut::expect(strcmp(t.description(), "desc") == 0);
    boost::ut::expect(strcmp(t.name(), "com.skaginn3x.Error.runtimeError") == 0);
    boost::ut::expect(t.get_errno() == 0);
  };
}
