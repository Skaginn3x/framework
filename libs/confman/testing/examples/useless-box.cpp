// Used as a tool to verify front-end behaviour see: https://github.com/Skaginn3x/framework/pull/386
// inspired by useless box
#include <chrono>
#include <cstdint>
#include <string>

#include <mp-units/systems/si/si.h>
#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/progbase.hpp>
#include <tfc/stx/glaze_meta.hpp>
#include <tfc/utils/units_glaze_meta.hpp>
#include <tfc/dbus/sd_bus.hpp>

namespace asio = boost::asio;

struct simple_config {
  tfc::confman::observable<int> a{ 1337 };
  tfc::confman::observable<std::string> b{ "Try to edit me" };
  tfc::confman::observable<bool> c{ false };
  tfc::confman::observable<std::vector<int>> d{ { 1, 2, 3, 4, 5 } };
  tfc::confman::observable<std::chrono::nanoseconds> sec{ std::chrono::nanoseconds{ 42 } };
  tfc::confman::observable<mp_units::quantity<mp_units::si::deci<mp_units::si::ampere>>> amper{ 11 * mp_units::si::ampere };
  struct glaze {
    using type = simple_config;
    // clang-format off
    static constexpr auto value{ glz::object(
        "a", &type::a,
        "b", &type::b,
        "c", &type::c,
        "d", &type::d,
        "sec", &type::sec,
        "amper", &type::amper
        ) };
    // clang-format on
    static constexpr auto name{ "useless box" };
  };
};

int main(int argc, char** argv) {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};
  auto dbus{ std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system()) };

  tfc::confman::config<simple_config> config{ dbus, "key" };
  // observe all variables and write default value back to it
  simple_config const default_config{};
  config->a.observe([&](auto new_value, auto) {
    if (new_value != default_config.a) {
      config.make_change()->a = default_config.a.value();
    }
  });
  config->b.observe([&](auto new_value, auto) {
    if (new_value != default_config.b) {
      config.make_change()->b = default_config.b.value();
    }
  });
  config->c.observe([&](auto new_value, auto) {
    if (new_value != default_config.c) {
      config.make_change()->c = default_config.c.value();
    }
  });
  config->d.observe([&](auto new_value, auto) {
    if (new_value != default_config.d) {
      config.make_change()->d = default_config.d.value();
    }
  });
  config->sec.observe([&](auto new_value, auto) {
    if (new_value != default_config.sec) {
      config.make_change()->sec = default_config.sec.value();
    }
  });
  config->amper.observe([&](auto new_value, auto) {
    if (new_value != default_config.amper) {
      config.make_change()->amper = default_config.amper.value();
    }
  });

  fmt::println("Schema is: {}", config.schema());
  fmt::println("Config is: {}", config.string());

  dbus->request_name(tfc::dbus::make_dbus_process_name().c_str());

  ctx.run();
  return 0;
}
