#include <fmt/core.h>
#include <mp-units/systems/si.h>
#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/ipc/details/item_glaze_meta.hpp>
#include <tfc/ipc/item.hpp>
#include <tfc/progbase.hpp>
#include <tfc/stx/glaze_meta.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace asio = boost::asio;

int main(int argc, char** argv) {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};
  auto dbus{ std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system()) };

  tfc::confman::config<tfc::confman::observable<tfc::ipc::item::item>> const config{ dbus, "key" };
  config->observe([](auto new_value, auto old_value) {
    fmt::print("new value: {}, old value: {}\n", new_value.to_json().value(), old_value.to_json().value());
  });

  fmt::print("Schema is: {}\n", config.schema());
  fmt::print("Config is: {}\n", config.string().value());

  dbus->request_name(tfc::dbus::make_dbus_process_name().c_str());

  ctx.run();
  return 0;
}
