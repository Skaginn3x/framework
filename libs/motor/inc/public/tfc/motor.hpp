#pragma once

#include <mp-units/systems/isq/isq.h>
#include <mp-units/systems/isq_angle/isq_angle.h>
#include <mp-units/systems/si/si.h>
#include <concepts>
#include <variant>

/**
 * This files contains a top level wrappers for motor abstractions.
 * Each motor implementation must adhere to the same user interface
 *
 * Each implementation can have a large degree of freedom for it's
 * own implementation.
 */

namespace tfc::motor {
using mp_units::QuantityOf;
using SpeedRatio = mp_units::ratio;

class motor_interface {
public:
  explicit motor_interface() = default;
  motor_interface(motor_interface&) = delete;
  motor_interface(motor_interface&&) = delete;

  void pump() {}
  void pump(QuantityOf<mp_units::isq::volume_flow_rate> auto) {}
  void pump(QuantityOf<mp_units::isq::volume_flow_rate> auto,
            QuantityOf<mp_units::isq::volume> auto,
            std::invocable<std::error_code> auto) {}
  void pump(QuantityOf<mp_units::isq::volume_flow_rate> auto,
            QuantityOf<mp_units::isq::time> auto,
            std::invocable<std::error_code> auto) {}
  void pump(QuantityOf<mp_units::isq::volume> auto, std::invocable<std::error_code> auto) {}
  void pump(QuantityOf<mp_units::isq::time> auto, std::invocable<std::error_code> auto) {}

  void convey() {}
  void convey(QuantityOf<mp_units::isq::velocity> auto) {}
  void convey(QuantityOf<mp_units::isq::velocity> auto,
              QuantityOf<mp_units::isq::length> auto,
              std::invocable<std::error_code> auto) {}
  void convey(QuantityOf<mp_units::isq::velocity> auto,
              QuantityOf<mp_units::isq::time> auto,
              std::invocable<std::error_code> auto) {}
  void convey(QuantityOf<mp_units::isq::length> auto, std::invocable<std::error_code> auto) {}
  void convey(QuantityOf<mp_units::isq::time> auto, std::invocable<std::error_code> auto) {}

  void rotate() {}
  void rotate(QuantityOf<mp_units::isq::angular_velocity> auto) {}
  void rotate(QuantityOf<mp_units::isq::angular_velocity> auto,
              QuantityOf<mp_units::angular::angle> auto,
              std::invocable<std::error_code> auto) {}
  void rotate(QuantityOf<mp_units::isq::angular_velocity> auto,
              QuantityOf<mp_units::isq::time> auto,
              std::invocable<std::error_code> auto) {}
  void rotate(QuantityOf<mp_units::angular::angle> auto, std::invocable<std::error_code> auto) {}
  void rotate(QuantityOf<mp_units::isq::time> auto, std::invocable<std::error_code> auto) {}

  void move(QuantityOf<mp_units::isq::length> auto){}
  void move(QuantityOf<mp_units::isq::length> auto, std::invocable<std::error_code> auto){}
  void move_home(std::invocable<std::error_code> auto){}

  void notify(QuantityOf<mp_units::isq::time> auto, std::invocable<std::error_code> auto){}
  void notify(QuantityOf<mp_units::angular::angle> auto, std::invocable<std::error_code> auto){}
  void notify(QuantityOf<mp_units::isq::length> auto, std::invocable<std::error_code> auto){}

  void stop(){}
  void stop(QuantityOf<mp_units::isq::time> auto){}
  void quick_stop(){}

  void run(){}
  void run(SpeedRatio){}

private:
  using implementations = std::variant<std::monostate>;
  implementations impl_{};
};
}  // namespace tfc::motor
