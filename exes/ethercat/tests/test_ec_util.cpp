#include <cstdint>
#include <type_traits>

#include <mp-units/systems/si/si.h>
#include <boost/ut.hpp>
#include <tfc/ec/devices/schneider/atv320.hpp>
#include <tfc/ec/devices/util.hpp>

namespace ut = boost::ut;
using tfc::ec::util::setting;

namespace example {
enum struct enum_value_e : std::int8_t { on = 1, off = 2 };
using enum_setting = setting<ecx::index_t{ 0x42, 0x43 }, "name", "desc", enum_value_e, enum_value_e::on>;
using trivial_type_setting = setting<ecx::index_t{ 0x42, 0x43 }, "name", "desc", uint8_t, 13>;
static_assert(sizeof(enum_setting) == 1);
static_assert(sizeof(trivial_type_setting) == 1);

// using std chrono
using chrono_test = setting<ecx::index_t{ 0x42, 0x42 }, "name", "desc", std::chrono::milliseconds, 60>;
static_assert(sizeof(std::chrono::milliseconds) == 8);
static_assert(std::is_same_v<chrono_test::type::rep, std::chrono::milliseconds::rep>);
static_assert(std::is_same_v<chrono_test::type::period, std::milli>);
static_assert(sizeof(chrono_test) == 8);

// using mp units

using mp_units::quantity;
using quantiy_test_t = quantity<mp_units::si::watt, uint32_t>;
using namespace mp_units::si::unit_symbols;
static_assert(sizeof(quantiy_test_t) == 4);
using mp_test = setting<ecx::index_t{ 0x42, 0x42 }, "name", "desc", quantiy_test_t, 60 * W>;
static_assert(std::is_same_v<mp_test::type::rep, uint32_t>);
static_assert(sizeof(mp_test) == 4);
}  // namespace example

auto main(int, char**) -> int {
  using ut::operator""_test;
  using ut::operator>>;
  using ut::expect;
  using ut::fatal;

  [[maybe_unused]] example::enum_setting const test1{};
  [[maybe_unused]] example::trivial_type_setting const test2{};
  [[maybe_unused]] example::chrono_test const test3{};
  [[maybe_unused]] example::mp_test const test4{};

  "setting to json"_test = []() {
    [[maybe_unused]] example::trivial_type_setting const test{};
    auto const json = glz::write_json(test);
    expect(json == "13") << "got: " << json;
    auto const exp = glz::read_json<example::trivial_type_setting>(json);
    expect(exp.has_value() >> fatal);
    expect(exp.value() == test);
  };

  "Test atv320 custom units"_test = []() {
    static_assert(std::chrono::seconds{ 1 } == tfc::ec::devices::schneider::deciseconds{ 10 });
    tfc::ec::devices::schneider::deciseconds an_hour = std::chrono::hours{ 1 };
    expect(an_hour == tfc::ec::devices::schneider::deciseconds{ 36000 });
  };
}
