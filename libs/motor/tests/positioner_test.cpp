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

  /// These are the tests for the tachometer
  /// Only one value can be updated at a time
  ///	new	new	old	old
  ///	pin1    pin2    pin1    pin2    Result
  ///	----	----	----	----	------
  ///	0       0	0       0	no movement
  ///	1       0	0       0	-1
  ///	0       1	0       0	+1
  ///	0       0	1       0	+1
  ///	0       0	0       1	-1
  ///	1       0	1       0	no movement
  ///	1       0	1       1	+1
  ///	0       1	0       1	no movement
  ///	0       1	1       1	-1
  ///	1       1	1       0	-1
  ///	1       1	0       1	+1
  ///	1       1	1       1	no movement

  "tachometer: 0"_test = [] {
    tfc::motor::detail::tachometer<> tacho{};

    expect(tacho.position_ == 0);

    tacho.first_tacho_update(false);
    tacho.second_tacho_update(false);

    expect(tacho.position_ == 0);
  };

  ///	0   1   |   0   0   ->  -1
  "tachometer: -1"_test = [] {
    tfc::motor::detail::tachometer<> tacho{};
    tacho.first_tacho_update(true);
    expect(tacho.position_ == -1);
  };

  ///	1   0   |   0   0   ->   +1
  "tachometer: +1"_test = [] {
    tfc::motor::detail::tachometer<> tacho{};
    tacho.second_tacho_update(true);
    expect(tacho.position_ == 1);
  };

  ///	0   0   |   0   1   ->   +1
  "tachometer: +1"_test = [] {
    tfc::motor::detail::tachometer<> tacho{};

    tacho.first_tacho_update(true);
    expect(tacho.position_ == -1);

    tacho.first_tacho_update(false);
    expect(tacho.position_ == 0);
  };

  ///	0   0   |   1   0   ->   -1
  "tachometer: -1"_test = [] {
    tfc::motor::detail::tachometer<> tacho{};

    tacho.second_tacho_update(true);
    expect(tacho.position_ == 1);

    tacho.second_tacho_update(false);
    expect(tacho.position_ == 0);
  };

  ///	0   1   |   0   1   ->   no movement
  "tachometer: 0"_test = [] {
    tfc::motor::detail::tachometer<> tacho{};

    tacho.first_tacho_update(true);
    expect(tacho.position_ == -1);

    tacho.first_tacho_update(true);
    expect(tacho.position_ == -1);
  };

  ///	0   1   |   1   1   ->   +1
  "tachometer: +1"_test = [] {
    tfc::motor::detail::tachometer<> tacho{};

    tacho.first_tacho_update(true);
    tacho.second_tacho_update(true);
    expect(tacho.position_ == -2) << tacho.position_;

    tacho.second_tacho_update(false);
    expect(tacho.position_ == -1);
  };

  ///	1   0   |   1   0   ->   no movement
  "tachometer: 0"_test = [] {
    tfc::motor::detail::tachometer<> tacho{};

    tacho.second_tacho_update(true);
    expect(tacho.position_ == 1);

    tacho.second_tacho_update(true);
    expect(tacho.position_ == 1);
  };

  ///	1   0   |   1   1   ->   -1
  "tachometer: -1"_test = [] {
    tfc::motor::detail::tachometer<> tacho{};

    tacho.first_tacho_update(true);
    tacho.second_tacho_update(true);
    expect(tacho.position_ == -2) << tacho.position_;

    tacho.first_tacho_update(false);
    expect(tacho.position_ == -3);
  };

  ///   1   1   |   0   1   ->   -1
  "tachometer: -1"_test = [] {
    tfc::motor::detail::tachometer<> tacho{};

    tacho.first_tacho_update(true);
    expect(tacho.position_ == -1) << tacho.position_;

    tacho.second_tacho_update(true);
    expect(tacho.position_ == -2) << tacho.position_;
  };

  ///   1   1   |   1   0   ->   +1
  "tachometer: +1"_test = [] {
    tfc::motor::detail::tachometer<> tacho{};

    tacho.second_tacho_update(true);
    expect(tacho.position_ == 1) << tacho.position_;

    tacho.first_tacho_update(true);
    expect(tacho.position_ == 2) << tacho.position_;
  };

  ///   1   1   |   1   1   ->   no movement
  "tachometer: 0"_test = [] {
    tfc::motor::detail::tachometer<> tacho{};

    tacho.first_tacho_update(true);
    tacho.second_tacho_update(true);
    expect(tacho.position_ == -2) << tacho.position_;

    tacho.first_tacho_update(true);
    tacho.second_tacho_update(true);
    expect(tacho.position_ == -2) << tacho.position_;
  };

  return EXIT_SUCCESS;
}
