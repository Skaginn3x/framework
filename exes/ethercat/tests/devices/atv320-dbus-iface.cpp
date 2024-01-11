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

using tfc::ec::devices::schneider::atv320::controller;
using tfc::ec::devices::schneider::atv320::input_t;
using tfc::ec::devices::schneider::atv320::micrometre_t;
using tfc::ec::devices::schneider::atv320::speedratio_t;

// auto get_good_status_stopped() -> input_t {
//   return input_t{
//     .current = 0,
//   };
// }
//
// auto get_good_status_running() -> input_t {
//   return input_t{
//     .current = 10,
//   };
// }

auto main(int argc, char const* const* argv) -> int {
  tfc::base::init(argc, argv);
  auto ctx = asio::io_context();
  auto dbus_connection = std::make_shared<sdbusplus::asio::connection>(ctx);

  auto slave_id = 0;
  auto ctrl = controller(dbus_connection, slave_id);

  "run_at_speedratio normal start and stop"_test = [&] {
    speedratio_t ratio = 1.0 * percent;
    bool ran = false;
    ctrl.run_at_speedratio(ratio, [&ctx, &ran](const std::error_code& err) -> void {
      expect(err.category() == tfc::motor::category());
      expect(tfc::motor::motor_enum(err));
      ctx.stop();
      ran = true;
    });
    steady_timer timer{ ctx };
    timer.expires_after(1ms);
    timer.async_wait([&](const std::error_code& timer_error) {
      expect(!timer_error);
      ctrl.stop([](const std::error_code& stop_error) { expect(!stop_error); });
    });
    ctx.run_for(std::chrono::milliseconds(5000ms));
    expect(ran);
  };

  "convey micrometre"_test = [&] {
    bool ran = false;
    ctrl.convey_micrometre(1000 * micrometre_t{}, [&ran](const std::error_code& err) {
      expect(!err);
      ran = true;
    });
    ctx.run_for(std::chrono::milliseconds(5000ms));
    expect(ran);
  };

  "more complex race conditions and competing invocations"_test = [&] {};
  return EXIT_SUCCESS;
}
