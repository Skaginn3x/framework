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
#include <tfc/motor/enums.hpp>
#include <tfc/stx/function_traits.hpp>

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
  explicit api(std::shared_ptr<sdbusplus::asio::connection> connection, std::string_view name, config_t default_config = {})
      : ctx_{ connection->get_io_context() }, impl_(), config_{ ctx_, name, default_config }, logger_{ name } {
    std::visit(
        [this, connection](auto& conf) {
          using conf_t = std::remove_cvref_t<decltype(conf)>;
          if constexpr (!std::same_as<std::monostate, conf_t>) {
            if constexpr (std::is_constructible_v<typename conf_t::impl, std::shared_ptr<sdbusplus::asio::connection>,
                                                  const conf_t&>) {
              impl_.emplace<typename conf_t::impl>(connection, conf);
            } else if constexpr (std::is_constructible_v<typename conf_t::impl, asio::io_context&, const conf_t&>) {
              impl_.emplace<typename conf_t::impl>(ctx_, conf);
            } else {
              []<bool flag = false> {
                static_assert(flag && "Type cannot be constructed");
              }
              ();
            }
          }
        },
        config_->value());
    config_->observe([this, connection](auto& new_v, auto& old_v) {
      // If there is the same motor type for the old and
      // the new it is the responsibility of the motor to
      // handle that change
      std::visit(
          [this, connection](auto& vst_new, auto& vst_old) {
            using conf_t = std::remove_cvref_t<decltype(vst_new)>;
            if constexpr (!std::same_as<decltype(vst_new), decltype(vst_old)> && !std::same_as<std::monostate, conf_t>) {
              logger_.info("Switching running motor config");
              if constexpr (std::is_constructible_v<typename conf_t::impl, std::shared_ptr<sdbusplus::asio::connection>,
                                                    conf_t>) {
                impl_.emplace<typename conf_t::impl>(connection, vst_new);
              } else if constexpr (std::is_constructible_v<typename conf_t::impl, asio::io_context&, conf_t&>) {
                impl_.emplace<typename conf_t::impl>(ctx_, vst_new);
              } else {
                []<bool flag = false> {
                  static_assert(flag && "Type cannot be constructed");
                }
                ();
              }
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
  /// \tparam travel_t feedback of travel. Underlying type is micrometre any given type will be truncated to that resolution
  /// \param token completion token to notify iff motor is in error state, or cancelled by another operation. And its travel
  /// during this convey. In normal operation the std::errc::operation_canceled feedback is the normal case because your user
  /// logic would have called some other operation making this operation stale.
  template <QuantityOf<mp_units::isq::length> travel_t = micrometre_t>
  auto convey(QuantityOf<mp_units::isq::velocity> auto velocity,
              asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type;

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
  /// error_code is set on server timeout
  auto needs_homing(asio::completion_token_for<void(std::error_code, bool)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, bool)>::return_type;

  /// \brief Notify when motor has reached the given position
  /// \param position to notify after, relative to current position
  /// \param token completion token to notify when motor has reached the given position
  template <QuantityOf<mp_units::isq::length> position_t>
  auto notify_after(position_t position, asio::completion_token_for<void(std::error_code, position_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, position_t)>::return_type;

  /// \brief Notify when motor has reached the given position
  /// \param position to notify after, relative to homing position
  /// \param token completion token to notify when motor has reached the given position
  template <QuantityOf<mp_units::isq::length> position_t>
  auto notify_from_home(position_t position, asio::completion_token_for<void(std::error_code, position_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, position_t)>::return_type;

  // void notify(QuantityOf<mp_units::isq::volume> auto, std::invocable<std::error_code> auto) {}

  /// \brief Send stop command to motor with default configured deceleration duration by the motor server
  /// \param token completion token to notify when motor has come to a stop
  auto stop(asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type;

  // todo implement more generally,
  // motor.deceleration(100ms).stop(token);
  /// \brief Send stop command to motor and decelerate given the duration
  /// \param token completion token to notify when motor has come to a stop
  /// \param deceleration_duration how long the motor should take to stop
  auto stop(QuantityOf<mp_units::isq::time> auto deceleration_duration,
            asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type;

  /// \brief Send quick stop command to motor and keep stopped until any other command is sent
  /// \param token completion token to notify when motor has come to a stop
  auto quick_stop(asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type;

  /// \brief Send run command to motor with default configured speedratio by the motor server
  /// \param token completion token to notify iff motor is in error state, or cancelled by another operation
  /// \param direction (optional) either forward or backward, forward being default speedratio and backward negative speedratio
  /// In normal operation the std::errc::operation_canceled feedback is the normal case because your user logic
  /// would have called some other operation making this operation stale.
  auto run(asio::completion_token_for<void(std::error_code)> auto&& token, direction_e direction = direction_e::forward) ->
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
  /// duration by the motor
  /// \param token completion token to notify if motor is in error state, cancelled by another
  /// \param direction (optional) either forward or backward, forward being default speedratio and backward negative speedratio
  /// operation, or finished successfully. In normal operation the notify will return success when time is reached and motor
  /// is stopped. Notify can return cancel if some other operation is called during the given time.
  auto run(QuantityOf<mp_units::isq::time> auto time, asio::completion_token_for<void(std::error_code)> auto&& token, direction_e direction = direction_e::forward) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type;


  /// \brief Try to reset any error on motor driver
  /// Can commonly be used when operation of the user logic is set to "running"
  auto reset(asio::completion_token_for<void(std::error_code)> auto&& token) ->
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
auto monostate_return(auto&& token) {
  return asio::async_compose<decltype(token), signature_t>(
      [](auto& self) {
        if constexpr (stx::function_traits<signature_t>::arity == 1) {
          self.complete(motor_error(errors::err_enum::no_motor_configured));
        } else if constexpr (stx::function_traits<signature_t>::arity == 2) {
          self.complete(motor_error(errors::err_enum::no_motor_configured), {});
        } else if constexpr (stx::function_traits<signature_t>::arity == 3) {
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

template <QuantityOf<mp_units::isq::length> travel_t>
auto api::convey(QuantityOf<mp_units::isq::velocity> auto velocity,
                 asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type {
  using signature_t = void(std::error_code, travel_t);
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

template <QuantityOf<mp_units::isq::length> position_t>
auto api::move(speedratio_t speedratio,
               position_t position,
               asio::completion_token_for<void(std::error_code, position_t)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, position_t)>::return_type {
  using signature_t = void(std::error_code, position_t);
  using namespace detail;
  return std::visit(overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                                [&](auto& motor_impl) {
                                  return motor_impl.move(speedratio, position, std::forward<decltype(token)>(token));
                                } },
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

template <QuantityOf<mp_units::isq::length> position_t>
auto api::notify_after(position_t position, asio::completion_token_for<void(std::error_code, position_t)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, position_t)>::return_type {
  using signature_t = void(std::error_code, position_t);
  using namespace detail;
  return std::visit(overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                                [&](auto& motor_impl) {
                                  return motor_impl.notify_after(position, std::forward<decltype(token)>(token));
                                } },
                    impl_);
}

template <QuantityOf<mp_units::isq::length> position_t>
auto api::notify_from_home(position_t position, asio::completion_token_for<void(std::error_code, position_t)> auto&& token)
    -> typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, position_t)>::return_type {
  using signature_t = void(std::error_code, position_t);
  using namespace detail;
  return std::visit(overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                                [&](auto& motor_impl) {
                                  return motor_impl.notify_from_home(position, std::forward<decltype(token)>(token));
                                } },
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

auto api::run(asio::completion_token_for<void(std::error_code)> auto&& token, direction_e direction) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
  using signature_t = void(std::error_code);
  using namespace detail;
  return std::visit(overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                                [&](auto& motor_impl) { return motor_impl.run(std::forward<decltype(token)>(token), direction); } },
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

auto api::run(QuantityOf<mp_units::isq::time> auto time, asio::completion_token_for<void(std::error_code)> auto&& token, direction_e direction) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
  using signature_t = void(std::error_code);
  using namespace detail;
  return std::visit(
      overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                  [&](auto& motor_impl) { return motor_impl.run(time, std::forward<decltype(token)>(token), direction); } },
      impl_);
}

auto api::reset(asio::completion_token_for<void(std::error_code)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
  using signature_t = void(std::error_code);
  using namespace detail;
  return std::visit(overloaded{ return_monostate<signature_t>(std::forward<decltype(token)>(token)),
                                [&](auto& motor_impl) { return motor_impl.reset(std::forward<decltype(token)>(token)); } },
                    impl_);
}

}  // namespace tfc::motor
