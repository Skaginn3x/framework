#include <boost/asio.hpp>
#include <boost/ut.hpp>
#include <tfc/motors/positioner.hpp>
#include <tfc/progbase.hpp>

using namespace mp_units::si::unit_symbols;

int main(int argc, char** argv) {
  using boost::ut::operator""_test;
  using boost::ut::expect;
  using tfc::motor::detail::circular_buffer;
  tfc::base::init(argc, argv);

  "circular_buffer_test moves pointer front when inserted to last item"_test = [] {
    static constexpr std::size_t len{ 10 };
    circular_buffer<int, 10> buff{};
    for (std::size_t i = 0; i < len + len; ++i) {  // try two rounds
      buff.emplace(i);
    }
    expect(buff.front() == len + len - 1) << buff.front();
  };

  //	new 	new	    old 	old
  //	pin2	pin1	pin2	pin1	Result
  //	----	----	----	----	------
  //	0	0	1	1	+2  (assume pin1 edges only)
  //	0	1	0	0	-1
  //	0	1	0	1	no movement
  //	0	1	1	0	-2  (assume pin1 edges only)
  //	0	1	1	1	+1
  //	1	0	0	0	+1
  //	1	0	0	1	-2  (assume pin1 edges only)
  //	1	0	1	0	no movement
  //	1	0	1	1	-1
  //	1	1	0	0	+2  (assume pin1 edges only)
  //	1	1	0	1	-1
  //	1	1	1	0	+1
  //	1	1	1	1	no movement

  //	new 	new	    old 	old
  //	pin2	pin1	pin2	pin1	Result
  //	----	----	----	----	------
  //	0	0	0	0	no movement
  "tachometer - no movement"_test = [] {
    tfc::motor::detail::tachometer<boost::asio::steady_timer::time_point, 1024> tacho{};

    expect(tacho.position_ == 0);

    // No movement
    tacho.first_tacho_update(false);
    tacho.second_tacho_update(false);

    expect(tacho.position_ == 0);
  };

  //	new 	new	    old 	old
  //	pin2	pin1	pin2	pin1	Result
  //	----	----	----	----	------
  //	0	0	0	1	+1
  "tachometer - +1"_test = [] {
    tfc::motor::detail::tachometer<boost::asio::steady_timer::time_point, 1024> tacho{};

    tacho.first_tacho_update(true);
    tacho.first_tacho_update(false);

    expect(tacho.position_ == 1);
  };

  //	new 	new	    old 	old
  //	pin2	pin1	pin2	pin1	Result
  //	----	----	----	----	------
  //	0	0	1	0	-1

  "tachometer - -1"_test = [] {
    tfc::motor::detail::tachometer<boost::asio::steady_timer::time_point, 1024> tacho{};

    tacho.second_tacho_update(true);

    expect(tacho.position_ == -1);

    tacho.first_tacho_update(false);
    tacho.second_tacho_update(false);

    expect(tacho.position_ == -2);
  };

  //	new 	new	    old 	old
  //	pin2	pin1	pin2	pin1	Result
  //	----	----	----	----	------
  //	0	0	1	1	+2  (assume pin1 edges only)
  //  "tachometer - +2"_test = [] {
  //    tfc::motor::detail::tachometer<boost::asio::steady_timer::time_point, 1024> tacho{};

  //    tacho.first_tacho_update(true);
  //    tacho.second_tacho_update(true);

  //    expect(tacho.position_ == 2);

  //    tacho.first_tacho_update(false);
  //    tacho.second_tacho_update(false);

  //    expect(tacho.position_ == 4);
  //  };

  return EXIT_SUCCESS;
}
