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
namespace ut = boost::ut;

using boost::ut::operator""_test;
using boost::ut::operator/;

struct operation_mode_test {
  asio::io_context ctx{};
  tfc::app_operation_mode app{ ctx };
  tfc::operation::interface lib {
    ctx
  };
  ~operation_mode_test() {
    std::error_code code;
    std::filesystem::remove_all(tfc::base::make_config_file_name("", ""), code);
  }
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  // todo depends on ipc-ruler running need to make a stub for tests
  ut::skip / "set mode"_test = [] {
    using tfc::operation::mode_e;

    operation_mode_test test{};
    sdbusplus::asio::connection dbus{ test.ctx, tfc::dbus::sd_bus_open_system() };
    tfc::confman::set_config(dbus, "state_machine",
                             tfc::operation::detail::storage{ .startup_time = std::chrono::milliseconds{ 3 } },
                             [](std::error_code) {});
    bool called{};
    test.lib.on_enter(mode_e::starting, [&called](mode_e new_mode, mode_e old_mode){
      called = true;
      ut::expect(new_mode == mode_e::starting);
      ut::expect(old_mode == mode_e::stopped);
    });
    test.lib.set(mode_e::starting);
    test.ctx.run_for(std::chrono::milliseconds(1000));
    ut::expect(called);
  };

  return EXIT_SUCCESS;
}
