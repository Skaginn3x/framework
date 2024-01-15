#pragma once
#include <mp-units/chrono.h>

#include <string>
#include <string_view>

#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>

#include <tfc/confman.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/dbus/sdbusplus_meta.hpp>
#include <tfc/motor/dbus_tags.hpp>
#include <tfc/motor/errors.hpp>
#include <tfc/stx/function_traits.hpp>

namespace tfc::motor::types {

namespace asio = boost::asio;
namespace method = dbus::method;
using mp_units::QuantityOf;
using micrometre_t = dbus::types::micrometre_t;
using microsecond_t = dbus::types::microsecond_t;
using speedratio_t = dbus::types::speedratio_t;

class atv320motor {
private:
  struct config {
    using impl = atv320motor;
    confman::observable<std::uint16_t> slave_id;
    struct glaze {
      using T = config;
      static constexpr auto value = glz::object("slave_id", &T::slave_id);
      static constexpr std::string_view name{ "atv320" };
    };
    auto operator==(const config&) const noexcept -> bool = default;
  };

  static constexpr std::string_view impl_name{ "atv320" };

  asio::io_context& ctx_;
  asio::steady_timer ping{ ctx_ };
  std::shared_ptr<sdbusplus::asio::connection> connection_{
    std::make_shared<sdbusplus::asio::connection>(ctx_, tfc::dbus::sd_bus_open_system())
  };
  logger::logger logger_{ "atv320motor" };
  uint16_t slave_id_{ 0 };
  std::string const service_name_{ dbus::service_name };
  std::string const path_{ dbus::path };
  std::string interface_name_{ dbus::make_interface_name(impl_name, slave_id_) };

  // Assemble the interface string and start ping-pong sequence.
  // Only set connected to true if there is an answer on the interface
  // and it is returning true. Indicating we have control over the motor.
  static constexpr bool long_living_ = false;
  void send_ping(uint16_t slave_id) {
    connection_->async_method_call(
        [this, slave_id](const std::error_code& err, bool response) {
          // It could be that we started before the ethercat network or a configuration
          // error has occured
          if (err) {
            // This error is returned, it is the users job to notify this failure and or deal with it.
            logger_.error("connect_to_motor_error: {}", err.message());
            response = false;
          }
          // New id our call chain is invalid
          if (slave_id_ != slave_id) {
            return;
          }
          connected_ = response;
          ping.expires_after(std::chrono::milliseconds(250));
          ping.async_wait([this, slave_id](const std::error_code& timer_fault) {
            // Deconstructed or canceled, either way we are done
            if (timer_fault) {
              return;
            }
            send_ping(slave_id);
          });
        },
        service_name_, path_, interface_name_, std::string{ method::ping }, long_living_);
  }
  bool connected_{ false };
  [[nodiscard]] std::error_code motor_seems_valid() const noexcept {
    if (!connected_) {
      return motor_error(errors::err_enum::motor_not_connected);
    }
    return {};
  }

public:
  using config_t = config;
  std::chrono::microseconds static constexpr method_call_timeout{ std::chrono::microseconds::max() };

  atv320motor(asio::io_context& ctx, const config_t& conf) : ctx_{ ctx } {
    slave_id_ = conf.slave_id.value();
    send_ping(slave_id_);
    conf.slave_id.observe([this](const std::uint16_t new_id, const std::uint16_t old_id) {
      logger_.warn("Configuration changed from {} to {}. It is not recomended to switch motors on a running system!", old_id,
                   new_id);
      connected_ = false;
      ping.cancel();
      slave_id_ = new_id;
      interface_name_ = dbus::make_interface_name(impl_name, slave_id_);
      send_ping(new_id);
    });
  }
  atv320motor(const atv320motor&) = delete;
  atv320motor(atv320motor&&) = delete;
  auto operator=(const atv320motor&) -> atv320motor& = delete;
  auto operator=(atv320motor&&) -> atv320motor& = delete;
  ~atv320motor() = default;

  template <typename signature_t = void(std::error_code)>
  auto convey(QuantityOf<mp_units::isq::velocity> auto, asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) { self.complete(motor_error(errors::err_enum::motor_method_not_implemented)); }, token);
  }

  template <QuantityOf<mp_units::isq::length> travel_t, typename signature_t = void(std::error_code, travel_t)>
  auto convey(QuantityOf<mp_units::isq::velocity> auto velocity,
              travel_t travel,
              asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    using mp_units::si::unit_symbols::s;
    return length_token_impl<signature_t>(
        std::string{ method::convey_micrometrepersecond_micrometre }, std::forward<decltype(token)>(token),
        velocity.force_in(micrometre_t::reference / s), travel.force_in(micrometre_t::reference));
  }

  template <QuantityOf<mp_units::isq::length> travel_t = micrometre_t,
            typename signature_t = void(std::error_code, travel_t)>
  auto convey(QuantityOf<mp_units::isq::velocity> auto velocity,
              QuantityOf<mp_units::isq::time> auto time,
              asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    using mp_units::si::unit_symbols::s;
    return length_token_impl<signature_t>(
        std::string{ method::convey_micrometrepersecond_microsecond }, std::forward<decltype(token)>(token),
        velocity.force_in(micrometre_t::reference / s), time.force_in(microsecond_t::reference));
  }

  template <QuantityOf<mp_units::isq::length> travel_t, typename signature_t = void(std::error_code, travel_t)>
  auto convey(travel_t travel, asio::completion_token_for<signature_t> auto&& token) {
    return length_token_impl<signature_t>(std::string{ method::convey_micrometre }, std::forward<decltype(token)>(token),
                                          travel.force_in(micrometre_t::reference));
  }

  template <QuantityOf<mp_units::isq::length> travel_t = micrometre_t,
            typename signature_t = void(std::error_code, travel_t)>
  auto convey(QuantityOf<mp_units::isq::time> auto time, asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return length_token_impl<signature_t>(std::string{ method::convey_microsecond }, std::forward<decltype(token)>(token),
                                          time.force_in(microsecond_t::reference));
  }

  template <QuantityOf<mp_units::isq::length> position_t, typename signature_t = void(std::error_code, position_t)>
  auto move(speedratio_t speedratio, position_t position, asio::completion_token_for<signature_t> auto&& token) {
    return length_token_impl<signature_t>(std::string{ method::move_speedratio_micrometre },
                                          std::forward<decltype(token)>(token), speedratio,
                                          position.force_in(micrometre_t::reference));
  }

  template <QuantityOf<mp_units::isq::length> position_t, typename signature_t = void(std::error_code, position_t)>
  auto move(position_t position, asio::completion_token_for<signature_t> auto&& token) {
    return length_token_impl<signature_t>(std::string{ method::move_micrometre }, std::forward<decltype(token)>(token),
                                          position.force_in(micrometre_t::reference));
  }

  template <typename signature_t = void(std::error_code)>
  auto move_home(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    return error_only_token_impl<signature_t>(std::string{ method::move_home }, std::forward<decltype(token)>(token));
  }

  template <typename signature_t = void(std::error_code, bool)>
  auto needs_homing(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    std::string method_name{ method::needs_homing };
    return asio::async_compose<decltype(token), signature_t>(
        [this, method_name](auto& self) {
          if (auto const sanity_check{ motor_seems_valid() }) {
            self.complete(sanity_check, {});
            return;
          }

          connection_->async_method_call(
              [this, &self, method_name](std::error_code const& err, errors::err_enum motor_err, bool needs_homing) {
                if (err) {
                  logger_.warn("{} failure: {}", method_name, err.message());
                  self.complete(err, needs_homing);
                  return;
                }
                using enum errors::err_enum;
                if (motor_err != success) {
                  logger_.warn("{} failure: {}", method_name, motor_err);
                }
                self.complete(motor_error(motor_err), needs_homing);
              },
              service_name_, path_, interface_name_, method_name);
        },
        token);
  }

  template <QuantityOf<mp_units::isq::length> position_t, typename signature_t = void(std::error_code, position_t)>
  auto notify_after(position_t position, asio::completion_token_for<signature_t> auto&& token) {
    return length_token_impl<signature_t>(std::string{ method::notify_after_micrometre },
                                          std::forward<decltype(token)>(token), position.force_in(micrometre_t::reference));
  }

  template <QuantityOf<mp_units::isq::length> position_t, typename signature_t = void(std::error_code, position_t)>
  auto notify_from_home(position_t position, asio::completion_token_for<signature_t> auto&& token) {
    return length_token_impl<signature_t>(std::string{ method::notify_from_home_micrometre },
                                          std::forward<decltype(token)>(token), position.force_in(micrometre_t::reference));
  }

  // void notify(QuantityOf<mp_units::isq::volume> auto, std::invocable<std::error_code> auto) {}

  template <typename signature_t = void(std::error_code)>
  auto stop(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    return error_only_token_impl<signature_t>(std::string{ method::stop }, std::forward<decltype(token)>(token));
  }
  template <typename signature_t = void(std::error_code)>
  auto stop(QuantityOf<mp_units::isq::time> auto, asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) { self.complete(motor_error(errors::err_enum::motor_method_not_implemented)); }, token);
  }

  template <typename signature_t = void(std::error_code)>
  auto quick_stop(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    return error_only_token_impl<signature_t>(std::string{ method::quick_stop }, std::forward<decltype(token)>(token));
  }

  template <typename signature_t = void(std::error_code)>
  auto run(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    return error_only_token_impl<signature_t>(std::string{ method::run }, std::forward<decltype(token)>(token));
  }

  template <typename signature_t = void(std::error_code)>
  auto run(speedratio_t speedratio, asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    return error_only_token_impl<signature_t>(std::string{ method::run_at_speedratio }, std::forward<decltype(token)>(token),
                                              speedratio);
  }

  template <typename signature_t = void(std::error_code)>
  auto run(speedratio_t speedratio,
           QuantityOf<mp_units::isq::time> auto time,
           asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return error_only_token_impl<signature_t>(std::string{ method::run_at_speedratio_microsecond },
                                              std::forward<decltype(token)>(token), speedratio,
                                              time.force_in(microsecond_t::reference));
  }

  template <typename signature_t = void(std::error_code)>
  auto run(QuantityOf<mp_units::isq::time> auto time, asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return error_only_token_impl<signature_t>(std::string{ method::run_at_speedratio }, std::forward<decltype(token)>(token),
                                              time.force_in(microsecond_t::reference));
  }

  template <typename signature_t = void(std::error_code)>
  auto reset(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    std::chrono::microseconds static constexpr timeout{ std::chrono::seconds{ 25 } };  // sdbus default
    return error_only_token_impl<signature_t, timeout>(std::string{ method::reset }, std::forward<decltype(token)>(token));
  }

private:
  template <typename signature_t>
  auto length_token_impl(std::string method_name, auto&& token, auto... args) {
    static_assert(stx::function_traits<signature_t>::arity == 2,
                  "signature_t must be of type void(std::error_code, QuantityOf<mp_units::isq::length> auto)");
    using second_arg_t = stx::function_traits_n_t<1, signature_t>;
    return asio::async_compose<decltype(token), signature_t>(
        [this, method_name, args...](auto& self) {
          if (auto const sanity_check{ motor_seems_valid() }) {
            self.complete(sanity_check, {});
            return;
          }

          connection_->async_method_call_timed(
              [this, &self, method_name](std::error_code const& err, errors::err_enum motor_err, micrometre_t length) {
                if (err) {
                  logger_.warn("{} failure: {}", method_name, err.message());
                  self.complete(err, length.force_in(second_arg_t::reference));
                  return;
                }
                using enum errors::err_enum;
                if (motor_err != success) {
                  logger_.warn("{} failure: {}", method_name, motor_err);
                }
                self.complete(motor_error(motor_err), length.force_in(second_arg_t::reference));
              },
              service_name_, path_, interface_name_, method_name, method_call_timeout.count(), args...);
        },
        token);
  }
  template <typename signature_t, std::chrono::microseconds const& timeout = method_call_timeout>
  auto error_only_token_impl(std::string method_name, auto&& token, auto... args) {
    static_assert(stx::function_traits<signature_t>::arity == 1, "signature_t must be of type void(std::error_code)");
    return asio::async_compose<decltype(token), signature_t>(
        [this, method_name, args...](auto& self) {
          if (auto const sanity_check{ motor_seems_valid() }) {
            self.complete(sanity_check);
            return;
          }

          connection_->async_method_call_timed(
              [this, &self, method_name](std::error_code const& err, errors::err_enum motor_err) {
                if (err) {
                  logger_.warn("{} failure: {}", method_name, err.message());
                  self.complete(err);
                  return;
                }
                using enum errors::err_enum;
                if (motor_err != success) {
                  logger_.warn("{} failure: {}", method_name, motor_err);
                }
                self.complete(motor_error(motor_err));
              },
              service_name_, path_, interface_name_, method_name, timeout.count(), args...);
        },
        token);
  }
};
}  // namespace tfc::motor::types
