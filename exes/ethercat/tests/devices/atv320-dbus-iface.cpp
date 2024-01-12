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

struct instance {
  asio::io_context ctx{asio::io_context()};
  std::shared_ptr<sdbusplus::asio::connection> dbus_connection {std::make_shared<sdbusplus::asio::connection>(ctx)};
  std::uint16_t slave_id{ 0 };
  controller ctrl{ controller(dbus_connection, slave_id) };
  std::array<bool, 10> ran {};
};

auto main(int, char const* const* argv) -> int {
  std::array<char const*, 4> args{ argv[0], "--log-level", "trace", "--stdout" };
  tfc::base::init(args.size(), args.data());

  // Good path tests
  "run_at_speedratio normal start and stop"_test = [&] {
    instance inst;
    speedratio_t ratio = 1.0 * percent;
    inst.ctrl.run_at_speedratio(ratio, [&inst](const std::error_code& err) -> void {
      expect(tfc::motor::motor_enum(err) == err_enum::operation_canceled) << err;
      inst.ctx.stop();
      inst.ran[0] = true;
    });
    steady_timer timer{ inst.ctx };
    timer.expires_after(1ms);
    timer.async_wait([&](const std::error_code& timer_error) {
      expect(!timer_error);
      inst.ctrl.stop([](const std::error_code& stop_error) { expect(!stop_error); });
    });
    inst.ctx.run_for(5ms);
    expect(inst.ran[0]);
  };

  "convey micrometre"_test = [&] {
    instance inst;
    inst.ctrl.convey_micrometre(1000 * micrometre_t::reference, [&inst](err_enum err, const micrometre_t moved) {
        expect(err == err_enum::success);
        expect(moved == 1000 * micrometre_t::reference);
        inst.ran[0] = true;
        inst.ctx.stop();
      });
      inst.ctrl.positioner().increment_position(1000 * micrometre_t::reference);
      inst.ctx.run();
      expect(inst.ran[0]);
    };

    "update status error detection"_test = [&] {
      instance inst;
      inst.ctrl.update_status(get_good_status_running());
    };
    return EXIT_SUCCESS;
  }
