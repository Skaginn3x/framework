
#include <tfc/motor.hpp>
#include <tfc/progbase.hpp>

namespace motor = tfc::motor;
using namespace mp_units::si::unit_symbols;  // NOLINT(*-build-using-namespace)

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);
  boost::asio::io_context ctx;
  motor::interface my_motor {
    ctx, "my_motor"
  };

  /// Liquid transport
  my_motor.pump(10 * (l / s));
  my_motor.pump(10 * (l / min), 10 * l, [](const std::error_code&) {});
  my_motor.pump(10 * (l / min), 10 * min, [](const std::error_code&) {});

  // Configured speed
  my_motor.pump();
  my_motor.pump(10 * l, [](const std::error_code&) {});
  my_motor.pump(10 * min, [](const std::error_code&) {});

  /// Linear transport
  my_motor.convey(10 * (m / s));
  my_motor.convey(10 * (m / s), 10 * m, [](const std::error_code&) {});
  my_motor.convey(10 * (m / s), 10 * min, [](const std::error_code&) {});

  // Configured speed
  my_motor.convey();
  my_motor.convey(10 * m, [](const std::error_code&) {});
  my_motor.convey(10 * min, [](const std::error_code&) {});

  /// Rotational transport
  // motor.rotate(1 * 360 * (deg / min));
  // motor.rotate(1 * 360 * (deg / min), 90 * deg, [](const std::error_code&) {});
  // motor.rotate(1 * 360 * (deg / min), 10 * min, [](const std::error_code&) {});

  /// All types
  my_motor.run(50);  // Run with SpeedRatio
  my_motor.stop();   // Stop freewheel

  // Absolute positioning
  my_motor.move(10 * m);
  my_motor.move(10 * m, [](const std::error_code&) {});
  my_motor.move_home([](const std::error_code&) {});

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
  my_motor.stop(100 * ms);
  my_motor.quick_stop();  // Stop the motor with quick stop

  /// Getting notified of a continously running motor
  my_motor.notify(10 * min, [](std::error_code const&) {});
  my_motor.notify(10 * l, [](std::error_code const&) {});
  my_motor.notify(10 * m, [](std::error_code const&) {});
  // motor.notify(10 * rad, [](std::error_code const&) {});

  /// Special cases
  my_motor.run(0);  // Stop the motor
  my_motor.run();   // Start the motor at configured speed

  // Calling convey, rotate or move
  my_motor.run(50);
  my_motor.run(-50);

  ctx.run();

  return EXIT_SUCCESS;
}
