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
using micrometre_t = dbus::types::micrometre_t;

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

  /// \brief Convey indefinetly or until cancelled at specified velocity
  /// \param token completion token to notify iff motor is in error state, or cancelled by another operation
  /// In normal operation the std::errc::operation_canceled feedback is the normal case because your user logic
  /// would have called some other operation making this operation stale.
  auto convey(QuantityOf<mp_units::isq::velocity> auto velocity,
              asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type;

  /// \brief Convey a specific length at the given linear velocity and quickly stop when reached
  /// \tparam travel_t deduced type of length to travel. Underlying type is micrometre any given type will be truncated to
  /// that resolution
  /// \param length to travel
  /// \param token completion token to notify when motor sends quick_stop command
  /// notification supplies travel_t with actual travel that took place
  /// \note that when the motor sends the quick_stop command and calls the token the motor is still moving
  template <QuantityOf<mp_units::isq::length> travel_t>
  auto convey(QuantityOf<mp_units::isq::velocity> auto velocity,
              travel_t length,
              asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type;

  /// \brief Convey for a specific time at the given linear velocity and quickly stop when reached
  /// \tparam travel_t feedback of travel. Underlying type is micrometre any given type will be truncated to that resolution
  /// \param time to travel
  /// \param token completion token to notify when motor sends quick_stop command
  /// notification supplies travel_t with the travel made during this time
  /// \note that when the motor sends the quick_stop command and calls the token the motor is still moving
  template <QuantityOf<mp_units::isq::length> travel_t = micrometre_t>
  auto convey(QuantityOf<mp_units::isq::velocity> auto velocity,
              QuantityOf<mp_units::isq::time> auto time,
              asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type;

  /// \brief Convey a specific distance at default speedratio and quickly stop when reached
  /// \tparam travel_t deduced type of length to travel. Underlying type is micrometre any given type will be truncated to
  /// that resolution
  /// \param length to travel
  /// \param token completion token to notify when motor sends quick_stop command
  /// notification supplies travel_t with actual travel that took place
  /// \note that when the motor sends the quick_stop command and calls the token the motor is still moving
  template <QuantityOf<mp_units::isq::length> travel_t>
  auto convey(travel_t length, asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type;

  /// \brief Convey for a specific time at default speedratio and quickly stop when reached
  /// \tparam travel_t feedback of travel. Underlying type is micrometre any given type will be truncated to that resolution
  /// \param time to travel
  /// \param token completion token to notify when motor sends quick_stop command
  /// notification supplies travel_t with the travel made during this time
  /// \note that when the motor sends the quick_stop command and calls the token the motor is still moving
  template <QuantityOf<mp_units::isq::time> time_t, QuantityOf<mp_units::isq::length> travel_t = micrometre_t>
  auto convey(time_t time, asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type;

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

  // TODO FINISH !!!!!!!!!! JBB
  /// \brief Move motor to the given absolute position relative to home position
  /// \param speedratio to move motor at [-100, 100]%
  /// \param position target to reach
  /// \param token completion token to notify when motor sends quick_stop command
  /// notification supplies position_t with the actual position relative to home where the motor is positioned
  /// \note Requires the motor being homed beforehand
  /// \note that when the motor sends the quick_stop command and calls the token the motor is still moving
  template <QuantityOf<mp_units::isq::length> position_t>
  auto move(speedratio_t speedratio,
            position_t position,
            asio::completion_token_for<void(std::error_code, position_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, position_t)>::return_type;

  /// \brief Move motor to the given absolute position relative to home position at default configured speedratio
  /// \param position target to reach
  /// \param token completion token to notify when motor sends quick_stop command
  /// notification supplies position_t with the actual position relative to home where the motor is positioned
  /// \note Requires the motor being homed beforehand
  /// \note that when the motor sends the quick_stop command and calls the token the motor is still moving
  template <QuantityOf<mp_units::isq::length> position_t>
  auto move(position_t position, asio::completion_token_for<void(std::error_code, position_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, position_t)>::return_type;

  /// TODO optional move and specify deceleration
  // template <QuantityOf<mp_units::isq::length> position_t, QuantityOf<mp_units::isq::time> decel_t>
  // auto move(position_t position, decel_t deceleration_duration, asio::completion_token_for<void(std::error_code,
  // position_t)> auto&& token);

  /// \brief move motor towards the home sensor on the configured homing travel speedratio
  /// \param token completion token to notify when homing is complete
  auto move_home(asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type;

  /// \brief poll whether the motor needs homing
  /// \param token notification with supplied flag indicating necissity of homing
  auto needs_homing(asio::completion_token_for<void(std::error_code, bool)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, bool)>::return_type;

  // todo notify_after, notify_at or notify_from_home
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
  /// would have called some other operation making this operation stale.
  auto run(asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type;

  /// \brief Send run command to motor with specified speedratio
  /// \param speedratio to run motor at [-100, 100]%
  /// \param token completion token to notify iff motor is in error state, or cancelled by another operation
  /// In normal operation the std::errc::operation_canceled feedback is the normal case because your user logic
  /// would have called some other operation making this one stale.
  auto run(speedratio_t speedratio, asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type;

  /// \brief Run motor for specific time
  /// \param speedratio to run motor at [-100, 100]%
  /// \param time duration to run motor for, will stop given the default configured deceleration duration by the motor
  /// \param token completion token to notify if motor is in error state, cancelled by another operation, or finished
  /// successfully. In normal operation the notify will return success when time is reached and motor is stopped. Notify can
  /// return cancel if some other operation is called during the given time.
  auto run(speedratio_t speedratio,
           QuantityOf<mp_units::isq::time> auto time,
           asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type;

  /// \brief Run motor for specific time
  /// \param time duration to run motor for at configured speedratio, will stop given the default configured deceleration
  /// duration by the motor \param token completion token to notify if motor is in error state, cancelled by another
  /// operation, or finished successfully. In normal operation the notify will return success when time is reached and motor
  /// is stopped. Notify can return cancel if some other operation is called during the given time.
  auto run(QuantityOf<mp_units::isq::time> auto time, asio::completion_token_for<void(std::error_code)> auto&& token) ->
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
template <typename signature_t>
struct function_traits;

template <typename return_t, typename... args_t>
struct function_traits<return_t(args_t...)> {
  static constexpr std::size_t arity = sizeof...(args_t);
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
  return [&](std::monostate) mutable { return monostate_return<signature_t>(std::forward<decltype(token)>(token)); };
}

template <typename... types_t>
struct overloaded : types_t... {
  using types_t::operator()...;
};
}  // namespace detail

auto api::convey(QuantityOf<mp_units::isq::velocity> auto velocity,
                 asio::completion_token_for<void(std::error_code)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
  using signature_t = void(std::error_code);
  using namespace detail;
  return std::visit(
      overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                  [&](auto& motor_impl) { return motor_impl.convey(velocity, std::forward<decltype(token)>(token)); } },
      impl_);
}

template <QuantityOf<mp_units::isq::length> travel_t>
auto api::convey(QuantityOf<mp_units::isq::velocity> auto velocity,
                 travel_t length,
                 asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type {
  using signature_t = void(std::error_code, travel_t);
  using namespace detail;
  return std::visit(overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                                [&](auto& motor_impl) {
                                  return motor_impl.convey(velocity, length, std::forward<decltype(token)>(token));
                                } },
                    impl_);
}

template <QuantityOf<mp_units::isq::length> travel_t>
auto api::convey(QuantityOf<mp_units::isq::velocity> auto velocity,
                 QuantityOf<mp_units::isq::time> auto time,
                 asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type {
  using signature_t = void(std::error_code, travel_t);
  using namespace detail;
  return std::visit(overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                                [&](auto& motor_impl) {
                                  return motor_impl.convey(velocity, time, std::forward<decltype(token)>(token));
                                } },
                    impl_);
}

template <QuantityOf<mp_units::isq::length> travel_t>
auto api::convey(travel_t length, asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type {
  using signature_t = void(std::error_code, travel_t);
  using namespace detail;
  return std::visit(
      overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                  [&](auto& motor_impl) { return motor_impl.convey(length, std::forward<decltype(token)>(token)); } },
      impl_);
}

template <QuantityOf<mp_units::isq::time> time_t, QuantityOf<mp_units::isq::length> travel_t>
auto api::convey(time_t time, asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type {
  using signature_t = void(std::error_code, travel_t);
  using namespace detail;
  return std::visit(
      overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                  [&](auto& motor_impl) { return motor_impl.convey(time, std::forward<decltype(token)>(token)); } },
      impl_);
}

template <QuantityOf<mp_units::isq::length> position_t>
auto api::move(position_t position, asio::completion_token_for<void(std::error_code, position_t)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, position_t)>::return_type {
  using signature_t = void(std::error_code, position_t);
  using namespace detail;
  return std::visit(
      overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                  [&](auto& motor_impl) { return motor_impl.move(position, std::forward<decltype(token)>(token)); } },
      impl_);
}

auto api::move_home(asio::completion_token_for<void(std::error_code)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
  using signature_t = void(std::error_code);
  using namespace detail;
  return std::visit(
      overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                  [&](auto& motor_impl) { return motor_impl.move_home(std::forward<decltype(token)>(token)); } },
      impl_);
}

auto api::needs_homing(asio::completion_token_for<void(std::error_code, bool)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, bool)>::return_type {
  using signature_t = void(std::error_code, bool);
  using namespace detail;
  return std::visit(
      overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                  [&](auto& motor_impl) { return motor_impl.needs_homing(std::forward<decltype(token)>(token)); } },
      impl_);
}

auto api::stop(asio::completion_token_for<void(std::error_code)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
  using signature_t = void(std::error_code);
  using namespace detail;
  return std::visit(overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                                [&](auto& motor_impl) { return motor_impl.stop(std::forward<decltype(token)>(token)); } },
                    impl_);
}

auto api::stop(QuantityOf<mp_units::isq::time> auto deceleration_duration,
               asio::completion_token_for<void(std::error_code)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
  using signature_t = void(std::error_code);
  using namespace detail;
  return std::visit(overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                                [&](auto& motor_impl) {
                                  return motor_impl.stop(deceleration_duration, std::forward<decltype(token)>(token));
                                } },
                    impl_);
}

auto api::quick_stop(asio::completion_token_for<void(std::error_code)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
  using signature_t = void(std::error_code);
  using namespace detail;
  return std::visit(
      overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                  [&](auto& motor_impl) mutable { return motor_impl.quick_stop(std::forward<decltype(token)>(token)); } },
      impl_);
}

auto api::brake(asio::completion_token_for<void(std::error_code)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
  using signature_t = void(std::error_code);
  using namespace detail;
  return std::visit(overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                                [&](auto& motor_impl) { return motor_impl.brake(std::forward<decltype(token)>(token)); } },
                    impl_);
}

auto api::run(asio::completion_token_for<void(std::error_code)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
  using signature_t = void(std::error_code);
  using namespace detail;
  return std::visit(overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                                [&](auto& motor_impl) { return motor_impl.run(std::forward<decltype(token)>(token)); } },
                    impl_);
}

auto api::run(speedratio_t speedratio, asio::completion_token_for<void(std::error_code)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
  using signature_t = void(std::error_code);
  using namespace detail;
  return std::visit(
      overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                  [&](auto& motor_impl) { return motor_impl.run(speedratio, std::forward<decltype(token)>(token)); } },
      impl_);
}

auto api::run(speedratio_t speedratio,
              QuantityOf<mp_units::isq::time> auto time,
              asio::completion_token_for<void(std::error_code)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
  using signature_t = void(std::error_code);
  using namespace detail;
  return std::visit(
      overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                  [&](auto& motor_impl) { return motor_impl.run(speedratio, time, std::forward<decltype(token)>(token)); } },
      impl_);
}

auto api::run(QuantityOf<mp_units::isq::time> auto time, asio::completion_token_for<void(std::error_code)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
  using signature_t = void(std::error_code);
  using namespace detail;
  return std::visit(
      overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                  [&](auto& motor_impl) { return motor_impl.run(time, std::forward<decltype(token)>(token)); } },
      impl_);
}

}  // namespace tfc::motor
