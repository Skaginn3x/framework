#include <fmt/format.h>
#include <units/isq/si/si.h>
#include <boost/ut.hpp>
#include <glaze/glaze.hpp>

#include <tfc/stx/glaze_meta.hpp>
#include <tfc/utils/units_common.hpp>
#include <tfc/utils/units_glaze_meta.hpp>
#include <tfc/utils/json_schema.hpp>

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
    ut::expect(json == R"({"value":42,"unit":"mm/s","dimension":"speed","ratio":{"numerator":1,"denominator":1000}})")
        << "got: " << json;
    [[maybe_unused]] auto bar = glz::read_json<target_velocity_t>(json);
    if (!bar.has_value()) {
      fmt::print("{}\n", glz::format_error(bar.error(), json));
    }
    ut::expect(glz::read_json<target_velocity_t>(json).has_value());
  };

  return EXIT_SUCCESS;
}
