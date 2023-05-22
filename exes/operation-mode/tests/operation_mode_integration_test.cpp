#include <boost/asio.hpp>
#include <boost/ut.hpp>
#include <sdbusplus/asio/property.hpp>

#include <tfc/confman/remote_change.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/operation_mode.hpp>
#include <tfc/progbase.hpp>
#include "app_operation_mode.hpp"
#include "state_machine.hpp"

namespace asio = boost::asio;

using boost::ut::operator""_test;

struct operation_mode_test {
  asio::io_context ctx{};
  tfc::app_operation_mode app{ ctx };
  tfc::operation::interface lib {
    ctx
  };
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  "set mode"_test = [] {
    operation_mode_test test{};
    sdbusplus::asio::connection dbus{ test.ctx, tfc::dbus::sd_bus_open_system() };
    tfc::confman::set_config(dbus, "state_machine",
                             tfc::operation::detail::storage{ .startup_time = std::chrono::milliseconds{ 3 } },
                             [](std::error_code) {});
    test.lib.set(tfc::operation::mode_e::starting);
    test.ctx.run_for(std::chrono::milliseconds(100));
  };

  return EXIT_SUCCESS;
}
