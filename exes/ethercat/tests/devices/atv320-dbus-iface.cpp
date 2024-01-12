#include <string_view>

#include <boost/asio.hpp>
#include <boost/ut.hpp>

#include <tfc/ec/devices/schneider/atv320/dbus-iface.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;
namespace ut = boost::ut;

using asio::steady_timer;

using std::chrono::operator""ms;
using std::string_view_literals::operator""sv;

using ut::operator""_test;
using ut::operator|;
using ut::expect;

using mp_units::percent;
using mp_units::si::micro;
using mp_units::si::metre;
using mp_units::si::unit_symbols::dHz;

using tfc::ec::devices::schneider::atv320::controller;
using tfc::ec::devices::schneider::atv320::input_t;
using tfc::ec::devices::schneider::atv320::micrometre_t;
using tfc::ec::devices::schneider::atv320::speedratio_t;
using tfc::motor::errors::err_enum;
using tfc::ec::devices::schneider::atv320::lft_e;
using tfc::ec::devices::schneider::atv320::hmis_e;

auto get_good_status_stopped() -> input_t {
  return input_t{
    .status_word = tfc::ec::cia_402::status_word{ .state_ready_to_switch_on = 1,
                                                  .state_switched_on = 1,
                                                  .voltage_enabled = 1,
                                                  .state_quick_stop = 1 },
    .frequency = 0 * dHz,
    .current = 0,
    .digital_inputs = 0x0000,
    .last_error = lft_e::no_fault,
    .drive_state = hmis_e::rdy,
  };
}

auto get_good_status_running() -> input_t {
  return input_t{
    .status_word = tfc::ec::cia_402::status_word{ .state_ready_to_switch_on = 1,
                                                  .state_switched_on = 1,
                                                  .state_operation_enabled = 1,
                                                  .voltage_enabled = 1,
                                                  .state_quick_stop = 1 },
    .frequency = 50 * dHz,
    .current = 10,
    .digital_inputs = 0x0000,
    .last_error = lft_e::no_fault,
    .drive_state = hmis_e::run,
  };
}

auto main(int argc, char const* const* argv) -> int {
  tfc::base::init(argc, argv);
  auto ctx = asio::io_context();
  auto dbus_connection = std::make_shared<sdbusplus::asio::connection>(ctx);

  auto slave_id = 0;
  auto ctrl = controller(dbus_connection, slave_id);

  // Good path tests
  "run_at_speedratio normal start and stop"_test = [&] {
    speedratio_t ratio = 1.0 * percent;
    bool ran = false;
    ctrl.run_at_speedratio(ratio, [&ctx, &ran](const std::error_code& err) -> void {
      expect(tfc::motor::motor_enum(err) == err_enum::operation_canceled) << err;
      ctx.stop();
      ran = true;
    });
    steady_timer timer{ ctx };
    timer.expires_after(1ms);
    timer.async_wait([&](const std::error_code& timer_error) {
      expect(!timer_error);
      ctrl.stop([](const std::error_code& stop_error) { expect(!stop_error); });
    });
    ctx.run_for(5ms);
    expect(ran);
  };

  "convey micrometre"_test = [&] {
    bool ran = false;
    ctrl.convey_micrometre(1000 * micrometre_t::reference, [&ran, &ctx](err_enum err, const micrometre_t moved) {
      expect(err == err_enum::success);
      expect(moved == 1000 * micrometre_t::reference);
      ran = true;
      ctx.stop();
    });
    ctrl.positioner().increment_position(1000 * micrometre_t::reference);
    ctx.run();
    expect(ran);
  };

  "update status error detection"_test = [&] {
    ctrl.update_status(get_good_status_running());
  };
  return EXIT_SUCCESS;
}
