#include <concepts>
#include <type_traits>
#include <variant>

#include <mp-units/systems/isq/isq.h>
#include <mp-units/systems/si/si.h>
#include <boost/asio.hpp>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/motor/atv320motor.hpp>
#include <tfc/motor/errors.hpp>
#include <tfc/motor/virtual_motor.hpp>

/**
 * This files contains a top level wrappers for motor abstractions.
 * Each motor implementation must adhere to the same user interface
 *
 * Each implementation can have a large degree of freedom for it's
 * own implementation.
 */

namespace tfc::motor {
namespace asio = boost::asio;
using mp_units::QuantityOf;
using SpeedRatio = mp_units::ratio;

class api {
public:
  using config_t =
      confman::observable<std::variant<std::monostate, types::virtual_motor::config_t, types::atv320motor::config_t>>;
  // Default initialize the motor as a printing motor
  explicit api(asio::io_context& ctx, std::string_view name, config_t default_config = {})
      : ctx_{ ctx }, impl_(), config_{ ctx_, name, default_config }, logger_{ name } {
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

  api(api&) = delete;
  api(api&&) = delete;

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

  [[nodiscard]] auto convey() -> std::error_code {
    return std::visit(
        [](auto& motor_impl_) -> std::error_code {
          if constexpr (std::same_as<std::monostate, std::remove_cvref_t<decltype(motor_impl_)>>) {
            return motor_error(errors::err_enum::no_motor_configured);
          } else if constexpr (!std::is_invocable_v<decltype(motor_impl_)>) {
            return motor_impl_.convey();
          }
        },
        impl_);
  }

  [[nodiscard]] auto convey(QuantityOf<mp_units::isq::velocity> auto vel) -> std::error_code {
    return std::visit(
        [&](auto& motor_impl_) -> std::error_code {
          if constexpr (!std::same_as<std::monostate, std::remove_cvref_t<decltype(motor_impl_)>>) {
            return motor_impl_.convey(vel);
          } else {
            return motor_error(errors::err_enum::no_motor_configured);
          }
        },
        impl_);
  }

  void convey(QuantityOf<mp_units::isq::velocity> auto vel,
              QuantityOf<mp_units::isq::length> auto length,
              std::invocable<std::error_code> auto cb) {
    std::visit(
        [&](auto& motor_impl_) {
          if constexpr (!std::same_as<std::monostate, std::remove_cvref_t<decltype(motor_impl_)>>) {
            motor_impl_.convey(vel, length, cb);
          } else {
            cb(motor_error(errors::err_enum::no_motor_configured));
          }
        },
        impl_);
  }

  void convey(QuantityOf<mp_units::isq::velocity> auto vel,
              QuantityOf<mp_units::isq::time> auto time,
              std::invocable<std::error_code> auto cb) {
    std::visit(
        [&](auto& motor_impl_) {
          if constexpr (!std::same_as<std::monostate, std::remove_cvref_t<decltype(motor_impl_)>>) {
            motor_impl_.convey(vel, time, cb);
          } else {
            cb(motor_error(errors::err_enum::no_motor_configured));
          }
        },
        impl_);
  }

  void convey(QuantityOf<mp_units::isq::length> auto length, std::invocable<std::error_code> auto cb) {
    std::visit(
        [&](auto& motor_impl_) {
          if constexpr (!std::same_as<std::monostate, std::remove_cvref_t<decltype(motor_impl_)>>) {
            motor_impl_.convey(length, cb);
          } else {
            cb(motor_error(errors::err_enum::no_motor_configured));
          }
        },
        impl_);
  }

  void convey(QuantityOf<mp_units::isq::time> auto time, std::invocable<std::error_code> auto cb) {
    std::visit(
        [&](auto& motor_impl_) {
          if constexpr (!std::same_as<std::monostate, std::remove_cvref_t<decltype(motor_impl_)>>) {
            motor_impl_.convey(time, cb);
          } else {
            cb(motor_error(errors::err_enum::no_motor_configured));
          }
        },
        impl_);
  }

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

  void move(QuantityOf<mp_units::isq::length> auto position, std::invocable<std::error_code> auto cb) {
    std::visit(
        [&](auto& motor_impl_) {
          if constexpr (!std::same_as<std::monostate, std::remove_cvref_t<decltype(motor_impl_)>>) {
            return motor_impl_.move(position, cb);
          } else {
            cb(motor_error(errors::err_enum::no_motor_configured));
          }
        },
        impl_);
  }

  void move_home(std::invocable<std::error_code> auto cb) {
    std::visit(
        [&](auto& motor_impl_) {
          if constexpr (!std::same_as<std::monostate, std::remove_cvref_t<decltype(motor_impl_)>>) {
            return motor_impl_.move_home(cb);
          } else {
            cb(motor_error(errors::err_enum::no_motor_configured));
          }
        },
        impl_);
  }

  [[nodiscard]] auto needs_homing() const -> std::expected<bool, std::error_code> {
    return std::visit(
        [&](auto& motor_impl_) -> std::expected<bool, std::error_code> {
          if constexpr (!std::same_as<std::monostate, std::remove_cvref_t<decltype(motor_impl_)>>) {
            return motor_impl_.needs_homing();
          } else {
            return std::unexpected(motor_error(errors::err_enum::no_motor_configured));
          }
        },
        impl_);
  }

  void notify(QuantityOf<mp_units::isq::time> auto, std::invocable<std::error_code> auto) {}

  void notify(QuantityOf<mp_units::isq::length> auto, std::invocable<std::error_code> auto) {}

  void notify(QuantityOf<mp_units::isq::volume> auto, std::invocable<std::error_code> auto) {}

  std::error_code stop() {
    return std::visit(
        [&](auto& motor_impl_) -> std::error_code {
          if constexpr (!std::same_as<std::monostate, std::remove_cvref_t<decltype(motor_impl_)>>) {
            return motor_impl_.stop();
          } else {
            return motor_error(errors::err_enum::no_motor_configured);
          }
        },
        impl_);
  }

  void stop(QuantityOf<mp_units::isq::time> auto) {}

  void quick_stop() {}

  void run() {}

  void run(SpeedRatio) {}

private:
  asio::io_context& ctx_;

  // TODO(omarhogni): Implement convey and move over ethercat motor
  using implementations = std::variant<std::monostate, types::virtual_motor, types::atv320motor>;
  implementations impl_;
  confman::config<config_t> config_;
  logger::logger logger_;
};
}  // namespace tfc::motor
