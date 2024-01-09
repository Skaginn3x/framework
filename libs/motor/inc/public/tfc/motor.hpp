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
using speedratio_t = dbus::types::speedratio_t;

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

  /// \brief Convey a specific distance at default speedratio and stop when done
  /// \tparam travel_t deduced type of length to travel underlying type is micrometre any given type will be truncated to
  /// that resolution \param length to travel \param token completion token to notify when motor sends quick_stop command
  /// Note that when the motor sends the quick_stop command the motor is still moving
  template <QuantityOf<mp_units::isq::length> travel_t>
  auto convey(travel_t length, asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type;

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
  template <QuantityOf<mp_units::isq::length> position_t>
  auto move(position_t position, asio::completion_token_for<void(std::error_code, position_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, position_t)>::return_type;

  /// \brief move motor towards the home sensor on the configured homing travel speedratio
  /// \param token completion token to notify when homing is complete
  auto move_home(asio::completion_token_for<void(std::error_code)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type;

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

  /// \brief Send stop command to motor with default configured deceleration duration by the motor server
  /// \param token completion token to notify when motor has come to a stop
  auto stop(asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type;

  /// \brief Send stop command to motor and decelerate given the duration
  /// \param token completion token to notify when motor has come to a stop
  /// \param deceleration_duration how long the motor should take to stop
  auto stop(QuantityOf<mp_units::isq::time> auto deceleration_duration,
            asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type;

  /// \brief Send quick stop command to motor and release brake when stopped
  /// \param token completion token to notify when motor has come to a stop
  auto quick_stop(asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type;

  /// \brief Send DC brake command to motor
  ///  Will keep motor braked until next command is executed
  /// \param token completion token to notify when motor has come to a stop
  auto brake(asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type;

  /// \brief Send run command to motor with default configured speedratio by the motor server
  /// \param token completion token to notify iff motor is in error state, or cancelled by another operation
  /// In normal operation the std::errc::operation_canceled feedback is the normal case because your user logic
  /// would have called some other operation making this one stale.
  auto run(asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type;

  /// \brief Send run command to motor with specified speedratio
  /// \param speedratio to run motor at [-100, 100]%
  /// \param token completion token to notify iff motor is in error state, or cancelled by another operation
  /// In normal operation the std::errc::operation_canceled feedback is the normal case because your user logic
  /// would have called some other operation making this one stale.
  auto run(speedratio_t speedratio, asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type;

private:
  asio::io_context& ctx_;

  // TODO(omarhogni): Implement convey and move over ethercat motor
  using implementations = std::variant<std::monostate, types::virtual_motor, types::atv320motor>;
  implementations impl_;
  confman::config<config_t> config_;
  logger::logger logger_;
};

namespace detail {
template <typename Signature>
struct function_traits;

template <typename Ret, typename... Args>
struct function_traits<Ret(Args...)> {
  static constexpr std::size_t arity = sizeof...(Args);
};

template <typename signature_t>
auto monostate_return(auto&& token) {
  return asio::async_compose<decltype(token), signature_t>(
      [](auto& self) {
        if constexpr (function_traits<signature_t>::arity == 1) {
          self.complete(motor_error(errors::err_enum::no_motor_configured));
        } else if constexpr (function_traits<signature_t>::arity == 2) {
          self.complete(motor_error(errors::err_enum::no_motor_configured), {});
        } else if constexpr (function_traits<signature_t>::arity == 3) {
          self.complete(motor_error(errors::err_enum::no_motor_configured), {}, {});
        } else {
          []<bool flag = false>() {
            static_assert(flag, "Implement more args or somehow populate automatically");
          }
          ();
        }
      },
      token);
}
template <typename signature_t>
auto return_monostate(auto&& token) {
  return [token_captured = std::forward<decltype(token)>(token)](std::monostate) mutable {
    return monostate_return<signature_t>(std::forward<decltype(token)>(token_captured));
  };
}

// helper type for the visitor #4
template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
}  // namespace detail

template <QuantityOf<mp_units::isq::length> travel_t>
auto api::convey(travel_t length, asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type {
  using signature_t = void(std::error_code, travel_t);
  using namespace detail;
  return std::visit(overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                                [length, token_captured = std::forward<decltype(token)>(token)](auto& motor_impl) mutable {
                                  return motor_impl.convey(length, std::forward<decltype(token)>(token_captured));
                                } },
                    impl_);
}

template <QuantityOf<mp_units::isq::length> position_t>
auto api::move(position_t position, asio::completion_token_for<void(std::error_code, position_t)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, position_t)>::return_type {
  using signature_t = void(std::error_code, position_t);
  using namespace detail;
  return std::visit(overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                                [position, token_captured = std::forward<decltype(token)>(token)](auto& motor_impl) mutable {
                                  return motor_impl.move(position, std::forward<decltype(token)>(token_captured));
                                } },
                    impl_);
}

auto api::move_home(asio::completion_token_for<void(std::error_code)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
  using signature_t = void(std::error_code);
  using namespace detail;
  return std::visit(overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                                [token_captured = std::forward<decltype(token)>(token)](auto& motor_impl) mutable {
                                  return motor_impl.move_home(std::forward<decltype(token)>(token_captured));
                                } },
                    impl_);
}

auto api::stop(asio::completion_token_for<void(std::error_code)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
  using signature_t = void(std::error_code);
  using namespace detail;
  return std::visit(overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                                [token_captured = std::forward<decltype(token)>(token)](auto& motor_impl) mutable {
                                  return motor_impl.stop(std::forward<decltype(token)>(token_captured));
                                } },
                    impl_);
}

auto api::stop(QuantityOf<mp_units::isq::time> auto deceleration_duration,
               asio::completion_token_for<void(std::error_code)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
  using signature_t = void(std::error_code);
  using namespace detail;
  return std::visit(
      overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                  [deceleration_duration, token_captured = std::forward<decltype(token)>(token)](auto& motor_impl) mutable {
                    return motor_impl.stop(deceleration_duration, std::forward<decltype(token)>(token_captured));
                  } },
      impl_);
}

auto api::quick_stop(asio::completion_token_for<void(std::error_code)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
  using signature_t = void(std::error_code);
  using namespace detail;
  return std::visit(overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                                [token_captured = std::forward<decltype(token)>(token)](auto& motor_impl) mutable {
                                  return motor_impl.quick_stop(std::forward<decltype(token)>(token_captured));
                                } },
                    impl_);
}

auto api::brake(asio::completion_token_for<void(std::error_code)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
  using signature_t = void(std::error_code);
  using namespace detail;
  return std::visit(overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                                [token_captured = std::forward<decltype(token)>(token)](auto& motor_impl) mutable {
                                  return motor_impl.brake(std::forward<decltype(token)>(token_captured));
                                } },
                    impl_);
}

auto api::run(asio::completion_token_for<void(std::error_code)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
  using signature_t = void(std::error_code);
  using namespace detail;
  return std::visit(overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                                [token_captured = std::forward<decltype(token)>(token)](auto& motor_impl) mutable {
                                  return motor_impl.run(std::forward<decltype(token)>(token_captured));
                                } },
                    impl_);
}

auto api::run(speedratio_t speedratio, asio::completion_token_for<void(std::error_code)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
  using signature_t = void(std::error_code);
  using namespace detail;
  return std::visit(
      overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                  [speedratio, token_captured = std::forward<decltype(token)>(token)](auto& motor_impl) mutable {
                    return motor_impl.run(speedratio, std::forward<decltype(token)>(token_captured));
                  } },
      impl_);
}

}  // namespace tfc::motor
