#include <boost/ut.hpp>
#include "tfc/dbus/exception.hpp"
#include "tfc/logger.hpp"

import std;
import tfc.base;

using std::string_view_literals::operator""sv;

auto main(int argc, char** argv) -> int {
  using boost::ut::operator""_test;
  using boost::ut::expect;

  tfc::base::init(argc, argv);

  "dbus runtime exception test"_test = []() {
    tfc::dbus::exception::runtime t("desc");

    boost::ut::expect(std::string_view(t.what()) == std::string_view("desc"));
    boost::ut::expect(std::string_view(t.description()) == std::string_view("desc"));
    boost::ut::expect(std::string_view(t.name()) == std::string_view("com.skaginn3x.Error.runtimeError"));
    boost::ut::expect(t.get_errno() == 0);
  };
}
