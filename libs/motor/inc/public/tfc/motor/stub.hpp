#pragma once
#include <string>

#include <mp-units/format.h>
#include <mp-units/ostream.h>
#include <mp-units/systems/isq/isq.h>
#include <mp-units/systems/si/si.h>
#include <boost/asio.hpp>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/motor/errors.hpp>
#include <tfc/motor/impl.hpp>
#include <tfc/utils/asio_condition_variable.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace tfc::motor::types {
using mp_units::QuantityOf;
using tfc::confman::observable;
using namespace mp_units::si::unit_symbols;
namespace asio = boost::asio;

/**
 * @brief stub motor class
 * @details This struct is only used for testing purposes.
 * none of the methods should be called in production.
 * This struct is also simple and does not rely on time by
 * default.
 */

struct stub {
  struct config {
    config() = default;
    using impl = stub;
    static constexpr std::string_view name{ "stub" };
    struct glaze {
      using T = config;
      static constexpr auto value = glz::object("name", &T::name);
      // Todo use tfc::json::schema meta and expose variant without this type to UI
      static constexpr std::string_view name{ "stub DONT PICK" };
    };
    auto operator==(const config&) const noexcept -> bool = default;
  };
  using config_t = config;
  explicit stub(asio::io_context& ctx, config_t) : tokens{ ctx.get_executor() }, logger_{ "stub" } {}

  template <QuantityOf<mp_units::isq::length> travel_t = micrometre_t,
            typename signature_t = void(std::error_code, travel_t)>
  auto move_convey_impl(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    signal.emit(asio::cancellation_type::all);
    running = true;
    return asio::async_compose<decltype(token), signature_t>(
        [this](auto& self) mutable {
          tokens.async_wait(asio::bind_cancellation_slot(
              signal.slot(), [this, self_m = std::move(self)](const std::error_code& err) mutable {
                // Try to be true to the behavior of the motor, if someone cancels this operation we should return that error
                if (err) {
                  self_m.complete(err, value_cast<typename travel_t::rep, travel_t::reference>(length));
                  return;
                }
                running = false;
                self_m.complete(code, value_cast<typename travel_t::rep, travel_t::reference>(length));
              }));
        },
        token);
  }

  template <QuantityOf<mp_units::isq::length> travel_t = micrometre_t,
            typename signature_t = void(std::error_code, travel_t)>
  auto convey(QuantityOf<mp_units::isq::velocity> auto, asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return move_convey_impl<travel_t, signature_t>(token);
  }

  template <QuantityOf<mp_units::isq::length> travel_t, typename signature_t = void(std::error_code, travel_t)>
  auto convey(QuantityOf<mp_units::isq::velocity> auto, travel_t, asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return move_convey_impl<travel_t, signature_t>(token);
  }

  template <QuantityOf<mp_units::isq::length> travel_t = micrometre_t,
            typename signature_t = void(std::error_code, travel_t)>
  auto convey(QuantityOf<mp_units::isq::velocity> auto,
              QuantityOf<mp_units::isq::time> auto,
              asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return move_convey_impl<travel_t, signature_t>(token);
  }

  template <QuantityOf<mp_units::isq::length> travel_t = micrometre_t,
            typename signature_t = void(std::error_code, travel_t)>
  auto convey(travel_t, asio::completion_token_for<signature_t> auto&& token) {
    return move_convey_impl<travel_t, signature_t>(token);
  }

  template <QuantityOf<mp_units::isq::length> position_t, typename signature_t = void(std::error_code, position_t)>
  auto move(speedratio_t, position_t, asio::completion_token_for<signature_t> auto&& token) {
    return move_convey_impl<position_t, signature_t>(token);
  }

  template <QuantityOf<mp_units::isq::length> position_t, typename signature_t = void(std::error_code, position_t)>
  auto move([[maybe_unused]] position_t position, asio::completion_token_for<signature_t> auto&& token) {
    return move_convey_impl<position_t, signature_t>(token);
  }

  template <typename signature_t = void(std::error_code)>
  auto move_home(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    signal.emit(asio::cancellation_type::all);
    running = true;
    return asio::async_compose<decltype(token), signature_t>(
        [this](auto& self) mutable {
          tokens.async_wait(asio::bind_cancellation_slot(
              signal.slot(), [this, self_m = std::move(self)](const std::error_code& err) mutable {
                // Try to be true to the behavior of the motor, if someone cancels this operation we should return that error
                if (err) {
                  self_m.complete(err);
                  return;
                }
                running = false;
                if (!code)
                  homed = true;
                self_m.complete(code);
              }));
        },
        token);
  }

  template <typename signature_t = void(std::error_code, bool)>
  auto needs_homing(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return asio::async_compose<decltype(token), signature_t>(
        [this](auto& self) mutable {
          tokens.async_wait([this, self_m = std::move(self)](const std::error_code&) mutable {
            // Try to be true to the behavior of the motor, if someone cancels this operation we should return that error
            if (!code)
              self_m.complete(code, !homed);
          });
        },
        token);
  }

  template <QuantityOf<mp_units::isq::length> position_t, typename signature_t = void(std::error_code, position_t)>
  auto notify_after(position_t, asio::completion_token_for<signature_t> auto&& token) {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) { self.complete(motor_error(errors::err_enum::motor_method_not_implemented), {}); }, token);
  }

  template <QuantityOf<mp_units::isq::length> position_t, typename signature_t = void(std::error_code, position_t)>
  auto notify_from_home(position_t, asio::completion_token_for<signature_t> auto&& token) {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) { self.complete(motor_error(errors::err_enum::motor_method_not_implemented), {}); }, token);
  }

  template <typename signature_t = void(std::error_code)>
  auto stop_impl(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    signal.emit(asio::cancellation_type::all);
    running = false;
    return asio::async_compose<decltype(token), signature_t>(
        [this](auto& self) mutable {
          tokens.async_wait(asio::bind_cancellation_slot(
              signal.slot(), [this, self_m = std::move(self)](const std::error_code& err) mutable {
                // Try to be true to the behavior of the motor, if someone cancels this operation we should return that error
                if (err) {
                  self_m.complete(err);
                  return;
                }
                self_m.complete(code);
              }));
        },
        token);
  }

  template <typename signature_t = void(std::error_code)>
  auto stop(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return stop_impl<signature_t>(token);
  }

  template <typename signature_t = void(std::error_code)>
  auto stop(QuantityOf<mp_units::isq::time> auto, asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return stop_impl<signature_t>(token);
  }

  template <typename signature_t = void(std::error_code)>
  auto quick_stop(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return stop_impl<signature_t>(token);
  }

  auto is_running() const -> bool { return running; }

  template <typename signature_t = void(std::error_code)>
  auto run_impl(asio::completion_token_for<signature_t> auto&& token, direction_e) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    signal.emit(asio::cancellation_type::all);
    running = true;
    return asio::async_compose<decltype(token), signature_t>(
        [this](auto& self) mutable {
          tokens.async_wait(asio::bind_cancellation_slot(
              signal.slot(), [this, self_m = std::move(self)](const std::error_code& err) mutable {
                // Try to be true to the behavior of the motor, if someone cancels this operation we should return that error
                if (err) {
                  self_m.complete(err);
                  return;
                }
                running = false;
                self_m.complete(code);
              }));
        },
        token);
  }

  template <typename signature_t = void(std::error_code)>
  auto run(asio::completion_token_for<signature_t> auto&& token, direction_e) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return run_impl(token, direction_e::forward);
  }

  template <typename signature_t = void(std::error_code)>
  auto run([[maybe_unused]] speedratio_t speedratio, asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return run_impl(token, direction_e::forward);
  }

  template <typename signature_t = void(std::error_code)>
  auto run(speedratio_t, QuantityOf<mp_units::isq::time> auto, asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return run_impl(token, direction_e::forward);
  }

  template <typename signature_t = void(std::error_code)>
  auto run(QuantityOf<mp_units::isq::time> auto, asio::completion_token_for<signature_t> auto&& token, direction_e) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return run_impl(token, direction_e::forward);
  }

  template <typename signature_t = void(std::error_code)>
  auto reset(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) { self.complete(motor_error(errors::err_enum::motor_method_not_implemented)); }, token);
  }

  // NOTE: signal needs to be declared before the condition_variable
  // for destruct order to be correct
  asio::cancellation_signal signal{};  // Used for canceling async operations

  // User logic callbacks for motor procedures
  tfc::asio::condition_variable tokens;
  logger::logger logger_;

  // Values to return
  std::error_code code{};
  micrometre_t length{};  // Used both for position and travel
  bool running{ false };
  bool homed{ false };
};
}  // namespace tfc::motor::types
