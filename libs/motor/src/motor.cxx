module;

#include <mp-units/systems/isq/isq.h>
#include <mp-units/systems/isq_angle/isq_angle.h>
#include <mp-units/systems/si/si.h>
#include <boost/asio.hpp>
#include <concepts>
#include <variant>

#include <tfc/confman.hpp>
#include "ethercat_motor.hpp"
#include "virtual_motor.hpp"

export module tfc.motor;

/**
 * This files contains a top level wrappers for motor abstractions.
 * Each motor implementation must adhere to the same user interface
 *
 * Each implementation can have a large degree of freedom for it's
 * own implementation.
 */

export namespace tfc::motor {

namespace asio = boost::asio;
using mp_units::QuantityOf;
using SpeedRatio = mp_units::ratio;

class interface {
public:
  // Default initialize the motor as a printing motor
  explicit interface(asio::io_context& ctx, std::string_view name)
      : ctx_{ ctx }, impl_(), config_{ ctx_, name }, logger_{ name } {
    std::visit(
        [this](auto& conf) {
          using conf_t = std::remove_cvref_t<decltype(conf)>;
          if constexpr (!std::same_as<std::monostate, conf_t>) {
            impl_.emplace<typename conf_t::impl>(ctx_, conf);
          }
        },
        config_->value());
    config_->observe([this](auto& new_v, auto& old_v) {
      // If there is the same motor type for the old and
      // the new it is the responsibility of the motor to
      // handle that change
      std::visit(
          [this](auto& vst_new, auto& vst_old) {
            using conf_t = std::remove_cvref_t<decltype(vst_new)>;
            if constexpr (!std::same_as<decltype(vst_new), decltype(vst_old)> && !std::same_as<std::monostate, conf_t>) {
              logger_.info("Switching running motor config");
              impl_.emplace<typename conf_t::impl>(ctx_, vst_new);
            }
          },
          new_v, old_v);
    });
  }
  interface(interface&) = delete;
  interface(interface&&) = delete;

  void pump() {
    return std::visit(
        [](auto& motor_impl_) {
          if constexpr (!std::same_as<std::monostate, std::remove_cvref_t<decltype(motor_impl_)>>) {
            motor_impl_.pump();
          }
        },
        impl_);
  }
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

  // TODO: rotate api
  // void rotate() {}
  // void rotate(QuantityOf<mp_units::isq::angular_velocity> auto) {}
  // void rotate(QuantityOf<mp_units::isq::angular_velocity> auto,
  //             QuantityOf<mp_units::angular::angle> auto,
  //             std::invocable<std::error_code> auto) {}
  // void rotate(QuantityOf<mp_units::isq::angular_velocity> auto,
  //             QuantityOf<mp_units::isq::time> auto,
  //             std::invocable<std::error_code> auto) {}
  // void rotate(QuantityOf<mp_units::angular::angle> auto, std::invocable<std::error_code> auto) {}
  // void rotate(QuantityOf<mp_units::isq::time> auto, std::invocable<std::error_code> auto) {}

  void move(QuantityOf<mp_units::isq::length> auto) {}
  void move(QuantityOf<mp_units::isq::length> auto, std::invocable<std::error_code> auto) {}
  void move_home(std::invocable<std::error_code> auto) {}

  void notify(QuantityOf<mp_units::isq::time> auto, std::invocable<std::error_code> auto) {}
  void notify(QuantityOf<mp_units::isq::length> auto, std::invocable<std::error_code> auto) {}
  void notify(QuantityOf<mp_units::isq::volume> auto, std::invocable<std::error_code> auto) {}

  void stop() {}
  void stop(QuantityOf<mp_units::isq::time> auto) {}
  void quick_stop() {}

  void run() {}
  void run(SpeedRatio) {}

private:
  asio::io_context& ctx_;

  using implementations = std::variant<std::monostate, types::virtual_motor, types::ethercat_motor>;
  using config_t = std::variant<std::monostate, types::virtual_motor::config_t, types::ethercat_motor::config_t>;
  implementations impl_;
  confman::config<confman::observable<config_t>> config_;
  logger::logger logger_;
};
}  // namespace tfc::motor
