
#include <boost/ut.hpp>
#include <tfc/progbase.hpp>

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  using boost::ut::operator""_test;
  using boost::ut::expect;

  return static_cast<int>(boost::ut::cfg<>.run({ .report_errors = true }));
}
