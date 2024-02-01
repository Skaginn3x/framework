#pragma once
#include <string>

#include <boost/asio.hpp>

#include <mp-units/format.h>
#include <mp-units/ostream.h>
#include <mp-units/systems/isq/isq.h>
#include <mp-units/systems/si/si.h>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/motor/errors.hpp>
#include <tfc/motor/impl.hpp>
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

struct config {

};

struct stub {
  using config_t = config;
  explicit stub(asio::io_context&) : logger_{"stub"}{}

  ~stub() {}
  template <QuantityOf<mp_units::isq::length> travel_t = micrometre_t,
            typename signature_t = void(std::error_code, travel_t)>
  auto convey(QuantityOf<mp_units::isq::velocity> auto, asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) { self.complete(motor_error(errors::err_enum::motor_method_not_implemented), {}); }, token);
  }

  template <QuantityOf<mp_units::isq::length> travel_t, typename signature_t = void(std::error_code, travel_t)>
  auto convey(QuantityOf<mp_units::isq::velocity> auto, travel_t, asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) { self.complete(motor_error(errors::err_enum::motor_method_not_implemented), {}); }, token);
  }

  template <QuantityOf<mp_units::isq::length> travel_t = micrometre_t,
            typename signature_t = void(std::error_code, travel_t)>
  auto convey(QuantityOf<mp_units::isq::velocity> auto,
              QuantityOf<mp_units::isq::time> auto,
              asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) { self.complete(motor_error(errors::err_enum::motor_method_not_implemented), {}); }, token);
  }

  template <QuantityOf<mp_units::isq::length> travel_t, typename signature_t = void(std::error_code, travel_t)>
  auto convey(travel_t length, asio::completion_token_for<signature_t> auto&& token) {
    // todo
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self, std::error_code = {}, travel_t = 0 * travel_t::reference) {
          self.complete(motor_error(errors::err_enum::motor_method_not_implemented), 0 * travel_t::reference);
        },
        token);
  }

  template <QuantityOf<mp_units::isq::length> position_t, typename signature_t = void(std::error_code, position_t)>
  auto move(speedratio_t, position_t, asio::completion_token_for<signature_t> auto&& token) {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) { self.complete(motor_error(errors::err_enum::motor_method_not_implemented), {}); }, token);
  }

  template <QuantityOf<mp_units::isq::length> position_t, typename signature_t = void(std::error_code, position_t)>
  auto move([[maybe_unused]] position_t position, asio::completion_token_for<signature_t> auto&& token) {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) { self.complete(motor_error(errors::err_enum::motor_method_not_implemented), {}); }, token);
  }

  template <typename signature_t = void(std::error_code)>
  auto move_home(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) { self.complete(motor_error(errors::err_enum::motor_method_not_implemented)); }, token);
  }

  template <typename signature_t = void(std::error_code, bool)>
  auto needs_homing(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) { self.complete(motor_error(errors::err_enum::motor_method_not_implemented), {}); }, token);
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
  auto stop(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) { self.complete(motor_error(errors::err_enum::motor_method_not_implemented)); }, token);
  }

  template <typename signature_t = void(std::error_code)>
  auto stop(QuantityOf<mp_units::isq::time> auto, asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) { self.complete(motor_error(errors::err_enum::motor_method_not_implemented)); }, token);
  }

  template <typename signature_t = void(std::error_code)>
  auto quick_stop(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) { self.complete(motor_error(errors::err_enum::motor_method_not_implemented)); }, token);
  }

  auto is_running() const -> bool { return false; }

  template <typename signature_t = void(std::error_code)>
  auto run(asio::completion_token_for<signature_t> auto&& token, direction_e) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) { self.complete(motor_error(errors::err_enum::motor_method_not_implemented)); }, token);
  }

  template <typename signature_t = void(std::error_code)>
  auto run([[maybe_unused]] speedratio_t speedratio, asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) { self.complete(motor_error(errors::err_enum::motor_method_not_implemented)); }, token);
  }

  template <typename signature_t = void(std::error_code)>
  auto run(speedratio_t, QuantityOf<mp_units::isq::time> auto, asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) { self.complete(motor_error(errors::err_enum::motor_method_not_implemented)); }, token);
  }

  template <typename signature_t = void(std::error_code)>
  auto run(QuantityOf<mp_units::isq::time> auto, asio::completion_token_for<signature_t> auto&& token, direction_e) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) { self.complete(motor_error(errors::err_enum::motor_method_not_implemented)); }, token);
  }

  template <typename signature_t = void(std::error_code)>
  auto reset(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) { self.complete(motor_error(errors::err_enum::motor_method_not_implemented)); }, token);
  }

  tfc::logger::logger logger_;
};
}  // namespace tfc::motor::types
