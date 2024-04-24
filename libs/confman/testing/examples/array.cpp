#include <fmt/core.h>
#include <mp-units/systems/si/si.h>
#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/stx/glaze_meta.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace asio = boost::asio;

struct object_in_array {
  int a{};
  mp_units::quantity<mp_units::si::deci<mp_units::si::ampere>> amper{};
  struct glaze {
    using type = object_in_array;
    static constexpr auto value{
      glz::object("a_int", &type::a, "A int description", "amper", &type::amper, "amper description")
    };
    static constexpr auto name{ "object_in_array" };
  };
};

int main(int argc, char** argv) {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};
  auto dbus{ std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system()) };

  tfc::confman::config<std::vector<object_in_array>> const config{ dbus, "key" };

  fmt::print("Schema is: {}\n", config.schema());
  fmt::print("Config is: {}\n", config.string());

  dbus->request_name(tfc::dbus::make_dbus_process_name().c_str());

  ctx.run();
  return 0;
}
