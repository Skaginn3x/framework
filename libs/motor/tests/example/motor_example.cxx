#include <boost/asio.hpp>
#include <fmt/core.h>
#include <mp-units/systems/si/unit_symbols.h>
#include <sdbusplus/asio/connection.hpp>

#include <tfc/progbase.hpp>
#include <tfc/motor.hpp>

namespace motor = tfc::motor;
namespace asio = boost::asio;
using namespace mp_units::si::unit_symbols;  // NOLINT(*-build-using-namespace)
using mp_units::percent;

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);
  boost::asio::io_context ctx;
  std::shared_ptr<sdbusplus::asio::connection> dbus{ std::make_shared<sdbusplus::asio::connection>(ctx) };
  motor::api my_motor {
    dbus, "my_motor"
  };

  /// Liquid transport
  //my_motor.pump(10 * (l / s));
  //my_motor.pump(10 * (l / min), 10 * l, [](const std::error_code&) {});
  //my_motor.pump(10 * (l / min), 10 * min, [](const std::error_code&) {});

  std::error_code err;

  // Configured speed
  my_motor.pump();
  //my_motor.pump(10 * l, [](const std::error_code&) {});
  my_motor.pump(10 * min, [](const std::error_code&) {});

  /// Linear transport
  my_motor.convey(10 * (m / s), [](std::error_code error) {
    if (error) {
      fmt::println(stderr, "Error: {}\n", error.message());
    }
  });
  my_motor.convey(10 * (m / s), 10 * m, [](const std::error_code&, [[maybe_unused]] decltype(10 * m) actual_travel) {});
  my_motor.convey(10 * (m / s), 10 * min, [](const std::error_code&, [[maybe_unused]] motor::micrometre_t travel_during_time) {});

  // Configured speed
  // err =  my_motor.convey();
  my_motor.convey(10 * m, [](std::error_code, [[maybe_unused]] auto actual_travel) {});

  co_spawn(ctx, [&]() -> asio::awaitable<void> {
    auto const ret{ my_motor.convey(10 * m, asio::use_awaitable) };
    fmt::println("Travelled: {}", "todo");
    co_return;
  }, asio::detached);

  /// Rotational transport
  // motor.rotate(1 * 360 * (deg / min));
  // motor.rotate(1 * 360 * (deg / min), 90 * deg, [](const std::error_code&) {});
  // motor.rotate(1 * 360 * (deg / min), 10 * min, [](const std::error_code&) {});

  /// All types
  my_motor.run(50 * percent, [](std::error_code){});  // Run with SpeedRatio
  my_motor.stop([](std::error_code){});   // Stop freewheel

  // Absolute positioning relative to home position
  my_motor.move(10 * percent, 10 * m, [](std::error_code, auto){});
  my_motor.move(10 * m, [](const std::error_code&, auto) {});
  my_motor.move_home([](const std::error_code&) {});

  // Can be used to ask the motor if he has a good reference to his home point.
  my_motor.needs_homing([](std::error_code, [[maybe_unused]] bool needs_homing){});

  // Stop with deceleration specified
  // specifing a deceleration overloads
  // the configured deceleration but
  // it does not replace it!
  //
  // If a sequence of
  // m.run(50%);
  // m.stop(100ms);
  // m.run(50%);
  // m.stop(); <- This uses the configured deceleration
  my_motor.stop(100 * ms, [](std::error_code){});
  my_motor.quick_stop([](std::error_code){});  // Stop the motor with quick stop

  /// Getting notified of a continously running motor
  // my_motor.notify(10 * min, [](std::error_code const&) {});
  //my_motor.notify(10 * l, [](std::error_code const&) {});
  my_motor.notify_after(10 * m, [](std::error_code const&, decltype(10 * m)) {});
  // motor.notify(10 * rad, [](std::error_code const&) {});

  /// Special cases
  my_motor.run(0 * percent, [](std::error_code){});  // Stop the motor
  my_motor.run([](std::error_code){});   // Start the motor at configured speed

  // Calling convey, rotate or move
  my_motor.run(50 * percent, [](std::error_code){});
  my_motor.run(-50 * percent, [](std::error_code){});

  // Run for 1 second
  my_motor.run(50 * percent, 1 * s, [](std::error_code){});
  my_motor.run(1 * s, [](std::error_code){});  // Run for 1 second at configured speed
  // my_motor.deceleration(1 * s).run(50 * percent, 10 * s, [](std::error_code){}); // todo
  // my_motor.deceleration(1 * s).acceleration(1 * s).run(50 * percent, 10 * s, [](std::error_code){}); // todo

  // reset any motor driver error
  // returns no error on successful reset
  my_motor.reset([](std::error_code){});

  ctx.run();

  return EXIT_SUCCESS;
}
