
#include <tfc/motor.hpp>
#include <tfc/progbase.hpp>

using tfc::motor::motor_interface;
using namespace mp_units::si::unit_symbols;  // NOLINT(*-build-using-namespace)

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  motor_interface motor{};

  /// Liquid transport
  motor.pump(10 * (l / s));
  motor.pump(10 * (l / min), 10 * l, [](const std::error_code&) {});
  motor.pump(10 * (l / min), 10 * min, [](const std::error_code&) {});

  // Configured speed
  motor.pump();
  motor.pump(10 * l, [](const std::error_code&) {});
  motor.pump(10 * min, [](const std::error_code&) {});

  /// Linear transport
  motor.convey(10 * (m / s));
  motor.convey(10 * (m / s), 10 * m, [](const std::error_code&) {});
  motor.convey(10 * (m / s), 10 * min, [](const std::error_code&) {});

  // Configured speed
  motor.convey();
  motor.convey(10 * m, [](const std::error_code&) {});
  motor.convey(10 * min, [](const std::error_code&) {});

  /// Rotational transport
  motor.rotate(1 * 360 * (deg / min));
  motor.rotate(1 * 360 * (deg / min), 90 * deg, [](const std::error_code&) {});
  motor.rotate(1 * 360 * (deg / min), 10 * min, [](const std::error_code&) {});

  /// All types
  motor.run(50);  // Run with SpeedRatio
  motor.stop();   // Stop freewheel

  // Absolute positioning
  motor.move(10 * m);
  motor.move(10 * m, [](const std::error_code&) {}););
  motor.move_home([](const std::error_code&) {});

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
  motor.stop(100 * ms);
  motor.quick_stop();  // Stop the motor with quick stop

  /// Getting notified of a continously running motor
  motor.notify(10 * l, [](std::error_code const&) {});
  motor.notify(10 * m, [](std::error_code const&) {});
  motor.notify(10 * deg, [](std::error_code const&) {});

  /// Special cases
  motor.run(0);  // Stop the motor
  motor.run();     // Start the motor at configured speed

  // Calling convey, rotate or move
  motor.run(50);
  motor.run(-50);

  return EXIT_SUCCESS;
}
