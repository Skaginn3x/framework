#include <type_traits>
#include <cstdint>

#include <boost/ut.hpp>
#include <units/quantity.h>
#include <units/isq/si/si.h>

#include <tfc/ec/devices/util.hpp>

using tfc::ec::util::setting;

namespace example {
enum struct enum_value_e : std::int8_t { on = 1, off = 2 };
using enum_setting = setting<ecx::index_t{ 0x42, 0x42 }, "name", "desc", enum_value_e, enum_value_e::on>;
using trivial_type_setting = setting<ecx::index_t{ 0x42, 0x42 }, "name", "desc", enum_value_e, enum_value_e::on>;

// using std chrono
using chrono_test = setting<ecx::index_t{ 0x42, 0x42 }, "name", "desc", std::chrono::milliseconds, 60>;
static_assert(sizeof(std::chrono::milliseconds) == 8);
static_assert(std::is_same_v<chrono_test::type::rep, std::chrono::milliseconds::rep>);
static_assert(std::is_same_v<chrono_test::type::period, std::milli>);
static_assert(sizeof(chrono_test) == 8);

// using mp units

using units::quantity;
using units::isq::si::watt;
using units::isq::si::dim_power;
using quantiy_test_t = quantity<dim_power, watt, uint32_t>;
static_assert(sizeof(quantiy_test_t) == 4);
using mp_test = setting<ecx::index_t{ 0x42, 0x42 }, "name", "desc", quantiy_test_t, 60>;
static_assert(std::is_same_v<mp_test::type::rep, uint32_t>);
static_assert(std::is_same_v<mp_test::type::dimension, dim_power>);
static_assert(std::is_same_v<mp_test::type::unit, watt>);
static_assert(sizeof(mp_test) == 4);

}  // namespace example

auto main(int, char**) -> int {
  using boost::ut::operator""_test;
  using boost::ut::expect;

  [[maybe_unused]] example::enum_setting const test1{};
  [[maybe_unused]] example::trivial_type_setting const test2{};
  [[maybe_unused]] example::chrono_test const test3{};
  [[maybe_unused]] example::mp_test const test4{};

  "mapping function"_test = []() {
    expect(tfc::ec::util::map(10, 0, 10, 0, 20) == 20);
    expect(tfc::ec::util::map(500, 0, 1000, 0, 20) == 10);
  };
}
