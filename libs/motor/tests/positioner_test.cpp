#include <type_traits>
#include <mp-units/systems/si/si.h>

#include <boost/ut.hpp>
#include <tfc/progbase.hpp>
#include <tfc/motors/positioner.hpp>

using namespace mp_units::si::unit_symbols;

int main(int argc, char** argv) {
  using boost::ut::operator""_test;
  using boost::ut::expect;
  using tfc::motor::detail::circular_buffer;
  tfc::base::init(argc, argv);

  "circular_buffer_test moves pointer front when inserted to last item"_test = [] {
    static constexpr std::size_t len{ 10 };
    circular_buffer<int, 10> buff{};
    for (std::size_t i = 0; i < len + len; ++i) { // try two rounds
      buff.emplace(i);
    }
    expect(buff.front() == len + len - 1) << buff.front();
  };

  return EXIT_SUCCESS;
}
