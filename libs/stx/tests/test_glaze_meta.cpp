#include <fmt/format.h>
#include <mp-units/systems/international.h>
#include <mp-units/systems/isq.h>
#include <mp-units/systems/si.h>

#include <boost/ut.hpp>
#include <glaze/glaze.hpp>

#include <tfc/stx/basic_fixed_string.hpp>
#include <tfc/stx/glaze_meta.hpp>
#include <tfc/utils/json_schema.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace ut = boost::ut;
using ut::operator""_test;
using ut::fatal;
using ut::operator>>;

namespace compile_test {
using tfc::unit::dimension_name;
namespace si = mp_units::si;
using namespace mp_units::si::unit_symbols;
static_assert(dimension_name<si::metre>() == "length");
static_assert(dimension_name<si::hertz>() == "frequency");
static_assert(dimension_name<si::ampere>() == "current");
static_assert(dimension_name<si::volt>() == "potential");
static_assert(dimension_name<si::watt>() == "power");
static_assert(dimension_name<si::gram>() == "mass");
static_assert(dimension_name<si::litre>() == "volume");
static_assert(dimension_name<mp_units::angular::degree>() == "angle");
static_assert(dimension_name<mp_units::square(si::milli<si::metre>)>() == "area");
static_assert(dimension_name<mp_units::square(si::centi<si::metre>)>() == "area");
static_assert(dimension_name<km / h>() == "speed");
static_assert(dimension_name<km / mp_units::square(h)>() == "acceleration");
}  // namespace compile_test

using std::string_view_literals::operator""sv;

auto main() -> int {
  namespace si = mp_units::si;
  using namespace mp_units::si::unit_symbols;
  "chrono"_test = [] {
    using test_t = std::chrono::duration<uint16_t, std::deci>;
    test_t foo{ std::chrono::seconds(32) };
    auto const json{ glz::write_json(foo) };
    ut::expect(fatal(json.has_value()));
    ut::expect(json == "320") << "got: " << json.value();
    ut::expect(glz::read_json<test_t>(json.value()).value() == foo);
  };
  "mp"_test = [] {
    using namespace mp_units::si::unit_symbols;
    auto foo{ 42 * (km / h) };
    auto const json{ glz::write_json(foo) };
    ut::expect(fatal(json.has_value()));
    ut::expect(json.value() == "42") << "got: " << json.value();
    [[maybe_unused]] auto bar = glz::read_json<decltype(foo)>(json.value());
    if (!bar.has_value()) {
      fmt::print("{}\n", glz::format_error(bar.error(), json.value()));
    }
    ut::expect(glz::read_json<decltype(foo)>(json.value()).has_value());
  };
  "fixed_string_to_json"_test = [] {
    tfc::stx::basic_fixed_string foo{ "HelloWorld" };
    auto foo_json{ glz::write_json(foo) };
    ut::expect(fatal(foo_json.has_value()));
    ut::expect(foo_json.value() == R"("HelloWorld")") << glz::write_json(foo).value();
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
  "millisecond clock"_test = [] {
    auto now{ std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()) };
    auto json{ glz::write_json(now) };
    ut::expect(fatal(json.has_value()));
    ut::expect(glz::read_json<decltype(now)>(json.value()).value() == now);
  };
  return EXIT_SUCCESS;
}
