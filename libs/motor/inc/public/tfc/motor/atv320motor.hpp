#pragma once
#include <mp-units/chrono.h>

#include <string>
#include <string_view>

#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/dbus/sdbusplus_meta.hpp>
#include <tfc/motor/dbus_tags.hpp>
#include <tfc/motor/enums.hpp>
#include <tfc/motor/errors.hpp>
#include <tfc/stx/function_traits.hpp>

namespace tfc::motor::types {

namespace asio = boost::asio;
namespace method = dbus::method;
using mp_units::QuantityOf;
using micrometre_t = dbus::types::micrometre_t;
using microsecond_t = dbus::types::microsecond_t;
using speedratio_t = dbus::types::speedratio_t;
using velocity_t = dbus::types::velocity_t;

template <typename rep, auto ref>
static constexpr auto value_cast(mp_units::Quantity auto quantity) -> mp_units::quantity<ref, rep> {
  if constexpr (std::floating_point<typename decltype(quantity)::rep>) {
    return mp_units::value_cast<rep>(quantity.force_in(ref));
  }
  return mp_units::value_cast<rep>(quantity).force_in(ref);
}
static constexpr auto micrometre_cast(QuantityOf<mp_units::isq::length> auto length) -> micrometre_t {
  return value_cast<micrometre_t::rep, micrometre_t::reference>(length);
}
static constexpr auto microsecond_cast(QuantityOf<mp_units::isq::time> auto time) -> microsecond_t {
  return value_cast<microsecond_t::rep, microsecond_t::reference>(time);
}
static constexpr auto velocity_cast(QuantityOf<mp_units::isq::velocity> auto velocity) -> velocity_t {
  return value_cast<velocity_t::rep, velocity_t::reference>(velocity);
}

static_assert(123456789 * micrometre_t::reference == micrometre_cast(123.456789 * mp_units::si::metre));
static_assert(123000000 * micrometre_t::reference == micrometre_cast(123LL * mp_units::si::metre));
static_assert(123 * micrometre_t::reference == micrometre_cast(123456LL * mp_units::si::nano<mp_units::si::metre>));
static_assert(123456789 * microsecond_t::reference == microsecond_cast(123.456789 * mp_units::si::second));
static_assert(123000000 * microsecond_t::reference == microsecond_cast(123LL * mp_units::si::second));
static_assert(123 * microsecond_t::reference == microsecond_cast(123456LL * mp_units::si::nano<mp_units::si::second>));
static_assert(3600000000LL * microsecond_t::reference == microsecond_cast(60 * mp_units::si::minute));

class atv320motor {
private:
  struct config {
    using impl = atv320motor;
    confman::observable<std::uint16_t> slave_id;
    struct glaze {
      using T = config;
      static constexpr auto value = glz::object("slave_id", &T::slave_id);
      static constexpr std::string_view name{ "Schneider ATV320" };
    };
    auto operator==(const config&) const noexcept -> bool = default;
  };

  static constexpr std::string_view impl_name{ "atv320" };

  asio::io_context& ctx_;
  asio::steady_timer ping_{ ctx_ };
  static constexpr std::chrono::milliseconds ping_interval{ 250 };
  static constexpr std::chrono::microseconds ping_response_timeout{ std::chrono::milliseconds{ 200 } };
  std::shared_ptr<sdbusplus::asio::connection> connection_;
  uint16_t slave_id_{ 0 };
  logger::logger logger_{ fmt::format("atv320motor.{}", slave_id_) };
  std::string const service_name_{ dbus::service_name };
  std::string const path_{ dbus::path };
  std::string interface_name_{ dbus::make_interface_name(impl_name, slave_id_) };

  void on_ping_response(std::error_code const& err, bool response) {
    if (err) {
      // todo invalid request descriptor is a known error, but we should handle it better
      // it is when the interface name (motor) is not existent
      logger_.error("DBus ping response error: {}", err.message());
      response = false;
    }
    connected_ = response;
    ping_.cancel();
  }

  void on_ping_timeout(std::error_code const& err) {
    if (err == std::errc::operation_canceled) {
      // We got response, meaning we did not need to wait as long as this timeout
      // logger_.trace("Received response from slave: {}, is connected: {}", slave_id_, connected_);
      ping_.expires_after(ping_interval);
      ping_.async_wait(std::bind_front(&atv320motor::on_ping_timeout, this));
      return;
    } else if (err) {
      logger_.error("Ping timeout error: {}", err.message());
    } else {
      // In most cases normal but a backup plan if dbus timeout fails
    }
    ping_.expires_after(ping_interval);
    ping_.async_wait(std::bind_front(&atv320motor::on_ping_timeout, this));
    connection_->async_method_call_timed(
        [this](std::error_code const& method_err, bool resp) { this->on_ping_response(method_err, resp); }, service_name_,
        path_, interface_name_, std::string{ method::ping }, ping_response_timeout.count(), false);
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

  atv320motor(std::shared_ptr<sdbusplus::asio::connection> connection, const config_t& conf)
      : ctx_{ connection->get_io_context() }, connection_{ connection }, slave_id_{ conf.slave_id.value() } {
    on_ping_timeout({});
    conf.slave_id.observe([this](const std::uint16_t new_id, const std::uint16_t old_id) {
      logger_.warn("Configuration changed from {} to {}. It is not recomended to switch motors on a running system!", old_id,
                   new_id);
      connected_ = false;
      slave_id_ = new_id;
      interface_name_ = dbus::make_interface_name(impl_name, slave_id_);
      logger_ = logger::logger{ fmt::format("atv320motor.{}", slave_id_) };
    });
  }
  atv320motor(const atv320motor&) = delete;
  atv320motor(atv320motor&&) = delete;
  auto operator=(const atv320motor&) -> atv320motor& = delete;
  auto operator=(atv320motor&&) -> atv320motor& = delete;
  ~atv320motor() = default;

  auto connected() -> bool { return connected_; }
  template <QuantityOf<mp_units::isq::length> travel_t = micrometre_t,
            typename signature_t = void(std::error_code, travel_t)>
  auto convey(QuantityOf<mp_units::isq::velocity> auto, asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return asio::async_compose<decltype(token), signature_t>(
        [](auto& self) {
          self.complete(motor_error(errors::err_enum::motor_method_not_implemented), 0 * travel_t::reference);
        },
        token);
  }

  template <QuantityOf<mp_units::isq::length> travel_t, typename signature_t = void(std::error_code, travel_t)>
  auto convey(QuantityOf<mp_units::isq::velocity> auto velocity,
              travel_t travel,
              asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    using mp_units::si::unit_symbols::s;
    return length_token_impl<signature_t>(method::convey_micrometrepersecond_micrometre,
                                          std::forward<decltype(token)>(token), velocity_cast(velocity),
                                          micrometre_cast(travel));
  }

  template <QuantityOf<mp_units::isq::length> travel_t = micrometre_t,
            typename signature_t = void(std::error_code, travel_t)>
  auto convey(QuantityOf<mp_units::isq::velocity> auto velocity,
              QuantityOf<mp_units::isq::time> auto time,
              asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    using mp_units::si::unit_symbols::s;
    return length_token_impl<signature_t>(method::convey_micrometrepersecond_microsecond,
                                          std::forward<decltype(token)>(token), velocity_cast(velocity),
                                          microsecond_cast(time));
  }

  template <QuantityOf<mp_units::isq::length> travel_t, typename signature_t = void(std::error_code, travel_t)>
  auto convey(travel_t travel, asio::completion_token_for<signature_t> auto&& token) {
    return length_token_impl<signature_t>(method::convey_micrometre, std::forward<decltype(token)>(token),
                                          micrometre_cast(travel));
  }

  template <QuantityOf<mp_units::isq::length> position_t, typename signature_t = void(std::error_code, position_t)>
  auto move(speedratio_t speedratio, position_t position, asio::completion_token_for<signature_t> auto&& token) {
    return length_token_impl<signature_t>(method::move_speedratio_micrometre, std::forward<decltype(token)>(token),
                                          speedratio, micrometre_cast(position));
  }

  template <QuantityOf<mp_units::isq::length> position_t, typename signature_t = void(std::error_code, position_t)>
  auto move(position_t position, asio::completion_token_for<signature_t> auto&& token) {
    return length_token_impl<signature_t>(method::move_micrometre, std::forward<decltype(token)>(token),
                                          micrometre_cast(position));
  }

  template <typename signature_t = void(std::error_code)>
  auto move_home(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    return error_only_token_impl<signature_t>(method::move_home, std::forward<decltype(token)>(token));
  }

  template <typename signature_t = void(std::error_code, bool)>
  auto needs_homing(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return asio::async_compose<decltype(token), signature_t>(
        [this](auto& self) {
          if (auto const sanity_check{ motor_seems_valid() }) {
            self.complete(sanity_check, {});
            return;
          }

          connection_->async_method_call(
              [this, self_m = std::move(self)](std::error_code const& err, dbus::message::needs_homing msg) mutable {
                if (err) {
                  logger_.warn("{} dbus failure: {}", method::needs_homing, err.message());
                  self_m.complete(err, msg.needs_homing);
                  return;
                }
                using enum errors::err_enum;
                self_m.complete(motor_error(msg.err), msg.needs_homing);
              },
              service_name_, path_, interface_name_, std::string(method::needs_homing));
        },
        token);
  }

  template <QuantityOf<mp_units::isq::length> position_t, typename signature_t = void(std::error_code, position_t)>
  auto notify_after(position_t position, asio::completion_token_for<signature_t> auto&& token) {
    return length_token_impl<signature_t>(method::notify_after_micrometre, std::forward<decltype(token)>(token),
                                          micrometre_cast(position));
  }

  template <QuantityOf<mp_units::isq::length> position_t, typename signature_t = void(std::error_code, position_t)>
  auto notify_from_home(position_t position, asio::completion_token_for<signature_t> auto&& token) {
    return length_token_impl<signature_t>(method::notify_from_home_micrometre, std::forward<decltype(token)>(token),
                                          micrometre_cast(position));
  }

  // void notify(QuantityOf<mp_units::isq::volume> auto, std::invocable<std::error_code> auto) {}

  template <typename signature_t = void(std::error_code)>
  auto stop(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    return error_only_token_impl<signature_t>(method::stop, std::forward<decltype(token)>(token));
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
    return error_only_token_impl<signature_t>(method::quick_stop, std::forward<decltype(token)>(token));
  }

  template <typename signature_t = void(std::error_code)>
  auto run(asio::completion_token_for<signature_t> auto&& token, direction_e direction = direction_e::forward) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    return error_only_token_impl<signature_t>(method::run, std::forward<decltype(token)>(token), direction);
  }

  template <typename signature_t = void(std::error_code)>
  auto run(speedratio_t speedratio, asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    return error_only_token_impl<signature_t>(method::run_at_speedratio, std::forward<decltype(token)>(token), speedratio);
  }

  template <typename signature_t = void(std::error_code)>
  auto run(speedratio_t speedratio,
           QuantityOf<mp_units::isq::time> auto time,
           asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return error_only_token_impl<signature_t>(method::run_at_speedratio_microsecond, std::forward<decltype(token)>(token),
                                              speedratio, microsecond_cast(time));
  }

  template <typename signature_t = void(std::error_code)>
  auto run(QuantityOf<mp_units::isq::time> auto time,
           asio::completion_token_for<signature_t> auto&& token,
           direction_e direction = direction_e::forward) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    return error_only_token_impl<signature_t>(method::run_microsecond, std::forward<decltype(token)>(token),
                                              microsecond_cast(time), direction);
  }

  template <typename signature_t = void(std::error_code)>
  auto reset(asio::completion_token_for<signature_t> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, signature_t>::return_type {
    std::chrono::microseconds static constexpr timeout{ std::chrono::seconds{ 25 } };  // sdbus default
    return error_only_token_impl<signature_t, timeout>(method::reset, std::forward<decltype(token)>(token));
  }

private:
  template <typename signature_t>
  auto length_token_impl(std::string_view method_name, auto&& token, auto... args) {
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
              [this, self_m = std::move(self), method_name](std::error_code const& err,
                                                            dbus::message::length input) mutable {
                auto [motor_err, length] = input;
                if (err) {
                  logger_.warn("{} failure: {}", method_name, err.message());
                  self_m.complete(err, length.force_in(second_arg_t::reference));
                  return;
                }
                using enum errors::err_enum;
                self_m.complete(motor_error(motor_err), length.force_in(second_arg_t::reference));
              },
              service_name_, path_, interface_name_, std::string(method_name), method_call_timeout.count(), args...);
        },
        token);
  }
  template <typename signature_t, std::chrono::microseconds const& timeout = method_call_timeout>
  auto error_only_token_impl(std::string_view method_name, auto&& token, auto... args) {
    static_assert(stx::function_traits<signature_t>::arity == 1, "signature_t must be of type void(std::error_code)");
    return asio::async_compose<decltype(token), signature_t>(
        [this, method_name, args...](auto& self) {
          if (auto const sanity_check{ motor_seems_valid() }) {
            self.complete(sanity_check);
            return;
          }

          connection_->async_method_call_timed(
              [this, self_m = std::move(self), method_name](std::error_code const& err, errors::err_enum motor_err) mutable {
                if (err) {
                  logger_.warn("{} failure: {}", method_name, err.message());
                  self_m.complete(err);
                  return;
                }
                using enum errors::err_enum;
                self_m.complete(motor_error(motor_err));
              },
              service_name_, path_, interface_name_, std::string(method_name), timeout.count(), args...);
        },
        token);
  }
};
}  // namespace tfc::motor::types
