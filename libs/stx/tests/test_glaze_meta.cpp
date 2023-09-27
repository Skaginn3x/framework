#include <fmt/format.h>
#include <units/isq/si/si.h>
#include <boost/ut.hpp>
#include <glaze/glaze.hpp>

#include <tfc/stx/basic_fixed_string.hpp>
#include <tfc/stx/glaze_meta.hpp>
#include <tfc/utils/json_schema.hpp>
#include <tfc/utils/units_common.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace ut = boost::ut;
using ut::operator""_test;
using ut::fatal;
using ut::operator>>;

namespace compile_test {
using tfc::unit::dimension_name;
namespace si = units::isq::si;
static_assert(dimension_name<si::dim_length>() == "length");
static_assert(dimension_name<si::dim_time>() == "time");
static_assert(dimension_name<si::dim_area>() == "area");
static_assert(dimension_name<si::dim_volume>() == "volume");
static_assert(dimension_name<si::dim_speed>() == "speed");
static_assert(dimension_name<si::dim_angular_velocity>() == "angular_velocity");
static_assert(dimension_name<si::dim_acceleration>() == "acceleration");
static_assert(dimension_name<si::dim_angular_acceleration>() == "angular_acceleration");
static_assert(dimension_name<si::dim_capacitance>() == "capacitance");
static_assert(dimension_name<si::dim_conductance>() == "conductance");
static_assert(dimension_name<si::dim_energy>() == "energy");
static_assert(dimension_name<si::dim_force>() == "force");
static_assert(dimension_name<si::dim_frequency>() == "frequency");
static_assert(dimension_name<si::dim_heat_capacity>() == "heat_capacity");
static_assert(dimension_name<si::dim_mass>() == "mass");
static_assert(dimension_name<si::dim_voltage>() == "voltage");
static_assert(dimension_name<si::dim_electric_current>() == "electric_current");
static_assert(dimension_name<si::dim_inductance>() == "inductance");
static_assert(dimension_name<si::dim_power>() == "power");
static_assert(dimension_name<si::dim_resistance>() == "resistance");
static_assert(dimension_name<si::dim_pressure>() == "pressure");
static_assert(dimension_name<si::dim_torque>() == "torque");
static_assert(dimension_name<si::dim_luminance>() == "luminance");
}  // namespace compile_test

using std::string_view_literals::operator""sv;

auto main() -> int {
  "chrono"_test = [] {
    using test_t = std::chrono::duration<uint16_t, std::deci>;
    test_t foo{ std::chrono::seconds(32) };
    std::string const json{ glz::write_json(foo) };
    ut::expect(json == "320") << "got: " << json;
    ut::expect(glz::read_json<test_t>(json).value() == foo);
  };
  "mp"_test = [] {
    using target_velocity_t = units::quantity<units::isq::si::dim_speed, tfc::unit::millimetre_per_second, int32_t>;
    target_velocity_t foo{ 42 };
    std::string const json{ glz::write_json(foo) };
    ut::expect(json == "42") << "got: " << json;
    [[maybe_unused]] auto bar = glz::read_json<target_velocity_t>(json);
    if (!bar.has_value()) {
      fmt::print("{}\n", glz::format_error(bar.error(), json));
    }
    ut::expect(glz::read_json<target_velocity_t>(json).has_value());
  };
  "fixed_string_to_json"_test = [] {
    tfc::stx::basic_fixed_string foo{ "HelloWorld" };
    auto foo_json{ glz::write_json(foo) };
    ut::expect(foo_json == R"("HelloWorld")") << glz::write_json(foo);
  };
  "fixed_string_from_json"_test = [] {
    auto foo = glz::read_json<tfc::stx::basic_fixed_string<char, 5>>("\"Hello\"");
    ut::expect(foo.has_value());
    ut::expect(foo.value() == "Hello"sv);
  };
  "fixed_string_from_json_too_long_caps_value"_test = [] {
    auto foo = glz::read_json<tfc::stx::basic_fixed_string<char, 5>>("\"Helloooo\"");
    ut::expect(foo.has_value());
    ut::expect(foo.value() == "Hello"sv);
  };
  "fixed_string_from_json_short"_test =
      [] {
        auto foo = glz::read_json<tfc::stx::basic_fixed_string<char, 5>>("\"Hell\"");
        ut::expect(foo.has_value());
        ut::expect(foo.value() == "Hell\0"sv)
            << foo.value().view();  // fixed string will always contain fixed number of characters hence the zero
      };
  "steady clock should not be transposable"_test = [] {
    // transposing steady clock in json does not really make any sense
    static_assert(glz::name_v<std::chrono::steady_clock> == "glz::unknown");
  };
  "millisecond clock"_test = [] {
    auto now{ std::chrono::system_clock::now() };
    auto json{ glz::write_json(now) };
    ut::expect(glz::read_json<decltype(now)>(json).value() == now);
  };
  return EXIT_SUCCESS;
}
