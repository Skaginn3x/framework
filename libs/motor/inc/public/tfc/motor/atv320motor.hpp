#pragma once
#include <mp-units/chrono.h>

#include <string>
#include <string_view>

#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>

#include <tfc/confman.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/motor/dbus_tags.hpp>
#include <tfc/motor/errors.hpp>

namespace tfc::motor::types {

namespace asio = boost::asio;
namespace method = dbus::method;
using mp_units::QuantityOf;
using SpeedRatio = mp_units::ratio;  // todo revert to mp_units quantity of percent?
using micrometre_t = dbus::types::micrometre_t;

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
  void send_ping(uint16_t slave_id) {
    connection_->async_method_call(
        [this, slave_id](const std::error_code& err, bool response) {
          // It could be that we started before the ethercat network or a configuration
          // error has occured
          if (err) {
            // This error is returned, it is the users job to notify this failure and or deal with it.
            // logger_.error("connect_to_motor: {}", err.message());
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
        service_name_, path_, interface_name_, std::string{ method::ping });
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

  [[nodiscard]] auto convey() -> std::error_code {
    auto sanity_check = motor_seems_valid();
    if (sanity_check)
      return sanity_check;
    return motor_error(errors::err_enum::motor_not_implemented);
  }

  [[nodiscard]] auto convey(QuantityOf<mp_units::isq::velocity> auto) -> std::error_code {
    auto sanity_check = motor_seems_valid();
    if (sanity_check)
      return sanity_check;
    return motor_error(errors::err_enum::motor_not_implemented);
  }

  void convey(QuantityOf<mp_units::isq::velocity> auto,
              QuantityOf<mp_units::isq::length> auto,
              std::invocable<std::error_code> auto cb) {
    auto sanity_check = motor_seems_valid();
    if (sanity_check) {
      cb(sanity_check);
      return;
    }
    cb(motor_error(errors::err_enum::motor_not_implemented));
  }

  void convey(QuantityOf<mp_units::isq::velocity> auto,
              QuantityOf<mp_units::isq::time> auto,
              std::invocable<std::error_code> auto cb) {
    auto sanity_check = motor_seems_valid();
    if (sanity_check) {
      cb(sanity_check);
      return;
    }
    cb(motor_error(errors::err_enum::motor_not_implemented));
  }

  template <QuantityOf<mp_units::isq::length> displacement_t>
  void convey(displacement_t displacement, std::invocable<std::error_code, displacement_t> auto cb) {
    if (auto const sanity_check{ motor_seems_valid() }) {
      cb(sanity_check);
      return;
    }
    std::chrono::microseconds constexpr timeout{ std::chrono::hours{ 24 } };
    connection_->async_method_call_timed(
        [this, cb](std::error_code const& err, errors::err_enum motor_err, micrometre_t actual_displacement) {
          if (err) {
            logger_.warn("Convey failure: {}", err.message());
            cb(err, actual_displacement);
            return;
          }
          using enum errors::err_enum;
          if (motor_err != success) {
            logger_.warn("Convey failure: {}", motor_err);
            cb(motor_error(motor_err), actual_displacement);
            return;
          }
          cb({}, actual_displacement);
        },
        service_name_, path_, interface_name_, std::string{ method::run_at_speedratio }, timeout.count(), displacement);
  }

  void convey(QuantityOf<mp_units::isq::time> auto time, std::invocable<std::error_code> auto cb) {
    logger_.trace("TIME: {}", time);
    auto sanity_check = motor_seems_valid();
    if (sanity_check) {
      cb(sanity_check);
      return;
    }
    connection_->async_method_call(
        [this, time, cb](const std::error_code& run_err, bool response) {
          if (run_err || !response) {
            logger_.error("failed to run motor: {}", run_err.message());
            cb(motor_error(errors::err_enum::motor_general_error));
            return;
          }
          auto timer = std::make_shared<asio::steady_timer>(ctx_);
          timer->expires_after(std::chrono::duration_cast<std::chrono::nanoseconds>(mp_units::to_chrono_duration(time)));
          logger_.trace("TIME: {}", time);
          timer->async_wait([this, timer, cb](const std::error_code& timer_err) {
            if (timer_err)
              return;
            connection_->async_method_call(
                [&, cb](const std::error_code& stop_err, bool stop_response) {
                  if (stop_err || !stop_response) {
                    logger_.error("failed to stop motor: {}", stop_err.message());
                    cb(motor_error(errors::err_enum::motor_general_error));
                    return;
                  }
                  cb({});
                },
                service_name_, path_, interface_name_, std::string{ method::stop });
          });
        },
        service_name_, path_, interface_name_, std::string{ method::run_at_speedratio }, 100.0);
  }

  void move(QuantityOf<mp_units::isq::length> auto, std::invocable<std::error_code> auto cb) {
    auto sanity_check = motor_seems_valid();
    if (sanity_check) {
      cb(sanity_check);
      return;
    }
    cb(motor_error(errors::err_enum::motor_not_implemented));
  }

  void move_home(std::invocable<std::error_code> auto cb) {
    auto sanity_check = motor_seems_valid();
    if (sanity_check) {
      cb(sanity_check);
      return;
    }
    cb(motor_error(errors::err_enum::motor_not_implemented));
  }

  [[nodiscard]] auto needs_homing() const -> std::expected<bool, std::error_code> {
    auto sanity_check = motor_seems_valid();
    if (sanity_check)
      return std::unexpected(sanity_check);
    return std::unexpected(motor_error(errors::err_enum::motor_not_implemented));
  }

  void notify(QuantityOf<mp_units::isq::time> auto, std::invocable<std::error_code> auto) {}

  void notify(QuantityOf<mp_units::isq::length> auto, std::invocable<std::error_code> auto) {}

  void notify(QuantityOf<mp_units::isq::volume> auto, std::invocable<std::error_code> auto) {}

  std::error_code stop() {
    auto sanity_check = motor_seems_valid();
    if (sanity_check)
      return sanity_check;
    return motor_error(errors::err_enum::motor_not_implemented);
  }

  void stop(QuantityOf<mp_units::isq::time> auto) {}

  void quick_stop() {}

  void run() {}

  void run(SpeedRatio) {}
};
}  // namespace tfc::motor::types
