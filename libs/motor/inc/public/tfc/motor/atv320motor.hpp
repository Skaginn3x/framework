#pragma once
#include <mp-units/chrono.h>

#include <iostream>  //TODO: Remove
#include <string>
#include <string_view>

#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>

#include <tfc/confman.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/motor/errors.hpp>

namespace tfc::motor::types {

namespace asio = boost::asio;
using mp_units::QuantityOf;
using SpeedRatio = mp_units::ratio;

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

  asio::io_context& ctx_;
  asio::steady_timer ping{ ctx_ };
  std::shared_ptr<sdbusplus::asio::connection> connection_;
  logger::logger logger_{ "atv320motor" };
  uint16_t slave_id_{ 0 };
  // Assemble the interface string and start ping-pong sequence.
  // Only set connected to true if there is an answer on the interface
  // and it is returning true. Indicating we have control over the motor.
  void send_ping(uint16_t slave_id) {
    connection_->async_method_call(
        [this, slave_id](const std::error_code& err, bool response) {
          // It could be that we started before the ethercat network or a configuration
          // error has occured
          if (err) {
            logger_.error("connect_to_motor: {}", err.message());
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
        "com.skaginn3x.ethercat", fmt::format("/com/skaginn3x/atvmotor{}", slave_id), "com.skaginn3x.atvmotor", "ping");
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
  explicit atv320motor(boost::asio::io_context& ctx, const config_t& conf) : ctx_{ ctx } {
    connection_ = std::make_shared<sdbusplus::asio::connection>(ctx_, dbus::sd_bus_open_system());
    slave_id_ = conf.slave_id.value();
    send_ping(slave_id_);
    conf.slave_id.observe([this](const std::uint16_t new_id, const std::uint16_t old_id) {
      logger_.warn("Configuration changed from {} to {}. It is not recomended to switch motors on a running system!", old_id,
                   new_id);
      connected_ = false;
      ping.cancel();
      slave_id_ = new_id;
      send_ping(new_id);
    });
  }

  [[nodiscard]] auto convey() -> std::error_code {
    auto sanity_check = motor_seems_valid();
    if (!sanity_check)
      return sanity_check;
    return motor_error(errors::err_enum::motor_not_implemented);
  }

  [[nodiscard]] auto convey(QuantityOf<mp_units::isq::velocity> auto) -> std::error_code {
    auto sanity_check = motor_seems_valid();
    if (!sanity_check)
      return sanity_check;
    return motor_error(errors::err_enum::motor_not_implemented);
  }

  void convey(QuantityOf<mp_units::isq::velocity> auto,
              QuantityOf<mp_units::isq::length> auto,
              std::invocable<std::error_code> auto cb) {
    auto sanity_check = motor_seems_valid();
    if (!sanity_check) {
      cb(sanity_check);
      return;
    }
    cb(motor_error(errors::err_enum::motor_not_implemented));
  }

  void convey(QuantityOf<mp_units::isq::velocity> auto,
              QuantityOf<mp_units::isq::time> auto,
              std::invocable<std::error_code> auto cb) {
    auto sanity_check = motor_seems_valid();
    if (!sanity_check) {
      cb(sanity_check);
      return;
    }
    cb(motor_error(errors::err_enum::motor_not_implemented));
  }

  void convey(QuantityOf<mp_units::isq::length> auto, std::invocable<std::error_code> auto cb) {
    auto sanity_check = motor_seems_valid();
    if (!sanity_check) {
      cb(sanity_check);
      return;
    }
    cb(motor_error(errors::err_enum::motor_not_implemented));
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
            logger_.error("connect_to_motor: {}", run_err.message());
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
                [&](const std::error_code& stop_err, bool stop_response) {
                  if (stop_err || !stop_response) {
                    logger_.error("connect_to_motor: {}", stop_err.message());
                    cb(motor_error(errors::err_enum::motor_general_error));
                    return;
                  }
                  cb({});
                },
                "com.skaginn3x.ethercat", fmt::format("/com/skaginn3x/atvmotor{}", slave_id_), "com.skaginn3x.atvmotor",
                "stop");
          });
        },
        "com.skaginn3x.ethercat", fmt::format("/com/skaginn3x/atvmotor{}", slave_id_), "com.skaginn3x.atvmotor",
        "run_at_speedratio", 100.0);
  }

  void move(QuantityOf<mp_units::isq::length> auto, std::invocable<std::error_code> auto cb) {
    auto sanity_check = motor_seems_valid();
    if (!sanity_check) {
      cb(sanity_check);
      return;
    }
    cb(motor_error(errors::err_enum::motor_not_implemented));
  }

  void move_home(std::invocable<std::error_code> auto cb) {
    auto sanity_check = motor_seems_valid();
    if (!sanity_check) {
      cb(sanity_check);
      return;
    }
    cb(motor_error(errors::err_enum::motor_not_implemented));
  }

  [[nodiscard]] auto needs_homing() const -> std::expected<bool, std::error_code> {
    auto sanity_check = motor_seems_valid();
    if (!sanity_check)
      return std::unexpected(sanity_check);
    return std::unexpected(motor_error(errors::err_enum::motor_not_implemented));
  }

  void notify(QuantityOf<mp_units::isq::time> auto, std::invocable<std::error_code> auto) {}

  void notify(QuantityOf<mp_units::isq::length> auto, std::invocable<std::error_code> auto) {}

  void notify(QuantityOf<mp_units::isq::volume> auto, std::invocable<std::error_code> auto) {}

  std::error_code stop() {
    auto sanity_check = motor_seems_valid();
    if (!sanity_check)
      return sanity_check;
    return motor_error(errors::err_enum::motor_not_implemented);
  }

  void stop(QuantityOf<mp_units::isq::time> auto) {}

  void quick_stop() {}

  void run() {}

  void run(SpeedRatio) {}
};
}  // namespace tfc::motor::types
