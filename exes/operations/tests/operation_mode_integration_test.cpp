#include <boost/asio.hpp>
#include <boost/ut.hpp>
#include <sdbusplus/asio/property.hpp>

#include <tfc/confman/remote_change.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/mocks/ipc.hpp>
#include <tfc/operation_mode.hpp>
#include <tfc/progbase.hpp>

#include "details/app_operation_mode_impl.hpp"
#include "details/state_machine_owner_impl.hpp"

namespace asio = boost::asio;
namespace ut = boost::ut;

using boost::ut::operator""_test;

struct operation_mode_test {
  asio::io_context ctx{};
  tfc::app_operation_mode<tfc::ipc::mock_signal, tfc::ipc::mock_slot> app{ ctx };
  tfc::operation::interface lib{ ctx, "operation", tfc::dbus::make_dbus_process_name() };
  ~operation_mode_test() {
    std::error_code code;
    std::filesystem::remove_all(tfc::base::make_config_file_name("", ""), code);
  }
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  "set mode"_test = [] {
    using tfc::operation::mode_e;

    operation_mode_test test{};
    sdbusplus::asio::connection dbus{ test.ctx, tfc::dbus::sd_bus_open_system() };
    tfc::confman::set_config(dbus, tfc::dbus::make_dbus_process_name(), "state_machine",
                             tfc::operation::detail::storage{ .startup_time = std::chrono::milliseconds{ 3 } },
                             [](std::error_code) {});

    size_t call_count = 0;
    test.lib.on_enter(mode_e::starting, [&call_count](mode_e new_mode, mode_e old_mode) {
      call_count += 1;
      ut::expect(new_mode == mode_e::starting);
      ut::expect(old_mode == mode_e::stopped);
    });

    test.lib.on_leave(mode_e::starting, [&call_count](mode_e new_mode, mode_e old_mode) {
      call_count += 1;
      ut::expect(new_mode == mode_e::running);
      ut::expect(old_mode == mode_e::starting);
    });

    // Initial
    test.lib.on_enter(mode_e::stopped, [&call_count](mode_e new_mode, mode_e old_mode) {
      call_count += 1;
      ut::expect(new_mode == mode_e::stopped);
      ut::expect(old_mode == mode_e::unknown);
    });
    test.lib.on_leave(mode_e::unknown, [&call_count](mode_e new_mode, mode_e old_mode) {
      call_count += 1;
      ut::expect(new_mode == mode_e::stopped);
      ut::expect(old_mode == mode_e::unknown);
    });

    test.lib.on_enter(mode_e::running, [&call_count](mode_e new_mode, mode_e old_mode) {
      call_count += 1;
      ut::expect(new_mode == mode_e::running);
      ut::expect(old_mode == mode_e::starting);
    });
    test.lib.set(mode_e::starting);
    test.ctx.run_for(std::chrono::milliseconds(100));
    ut::expect(call_count == 5);
  };
  "stop w reason"_test = [] {
    using tfc::operation::mode_e;

    operation_mode_test test{};
    sdbusplus::asio::connection dbus{ test.ctx, tfc::dbus::sd_bus_open_system() };
    tfc::confman::set_config(dbus, tfc::dbus::make_dbus_process_name(), "state_machine",
                             tfc::operation::detail::storage{ .startup_time = std::chrono::milliseconds{ 3 } },
                             [](std::error_code) {});

    auto& sm = test.app.state_machine();
    EXPECT_CALL(sm.stop_reason_str_signal(), async_send_cb("Test reason", testing::_)).Times(1);
    size_t call_count = 0;
    test.lib.on_enter(mode_e::stopped, [&call_count](auto, auto) { call_count += 1; });
    test.lib.stop("Test reason");
    test.ctx.run_for(std::chrono::milliseconds(2));
    ut::expect(call_count == 1);

    EXPECT_CALL(sm.stop_reason_str_signal(), async_send_cb("", testing::_)).Times(1);
    test.lib.set(mode_e::running);
    test.ctx.run_for(std::chrono::milliseconds(2));
  };

  return EXIT_SUCCESS;
}
