#pragma once
#include <iostream> //TODO: Remove
#include <string_view>
#include <string>

#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>

#include <tfc/dbus/sd_bus.hpp>
#include <tfc/confman.hpp>

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
  std::shared_ptr<sdbusplus::asio::connection> connection_;
  logger::logger logger_{"atv320motor"};
  uint16_t slave_id_{0};
  void pong() {
    connection_->async_method_call([this](const std::error_code&) {
      logger_.trace("PING RESPONSE");
    }, "com.skaginn3x.ethercat", "/com/skaginn3x/atvmotor1","com.skaginn3x.atvmotor", "ping");
  }
public:
  using config_t = config;
  explicit atv320motor(boost::asio::io_context& ctx, const config_t& conf) : ctx_{ctx} {
    connection_ = std::make_shared<sdbusplus::asio::connection>(ctx_, dbus::sd_bus_open_system());
    conf.slave_id.observe([this](const std::uint16_t old_id, const std::uint16_t new_id) {
      logger_.trace("Configuration changed from {} to {}", old_id, new_id);

    });
  }

  [[nodiscard]] auto convey() -> std::error_code {
    return motor_error(errors::err_enum::motor_not_implemented);
  }

  [[nodiscard]] auto convey(QuantityOf<mp_units::isq::velocity> auto) -> std::error_code {
    return motor_error(errors::err_enum::motor_not_implemented);
  }

  void convey(QuantityOf<mp_units::isq::velocity> auto,
              QuantityOf<mp_units::isq::length> auto,
              std::invocable<std::error_code> auto cb) {
    cb(motor_error(errors::err_enum::motor_not_implemented));
  }

  void convey(QuantityOf<mp_units::isq::velocity> auto,
              QuantityOf<mp_units::isq::time> auto,
              std::invocable<std::error_code> auto cb) {
    cb(motor_error(errors::err_enum::motor_not_implemented));
  }

  void convey(QuantityOf<mp_units::isq::length> auto, std::invocable<std::error_code> auto cb) {
    cb(motor_error(errors::err_enum::motor_not_implemented));
  }

  void convey(QuantityOf<mp_units::isq::time> auto, std::invocable<std::error_code> auto cb) {
    cb(motor_error(errors::err_enum::motor_not_implemented));
  }

  void move(QuantityOf<mp_units::isq::length> auto, std::invocable<std::error_code> auto cb) {
    cb(motor_error(errors::err_enum::motor_not_implemented));
  }

  void move_home(std::invocable<std::error_code> auto cb) {
    cb(motor_error(errors::err_enum::motor_not_implemented));
  }

  [[nodiscard]] auto needs_homing() const -> std::expected<bool, std::error_code> {
    return std::unexpected(motor_error(errors::err_enum::motor_not_implemented));
  }

  void notify(QuantityOf<mp_units::isq::time> auto, std::invocable<std::error_code> auto) {}

  void notify(QuantityOf<mp_units::isq::length> auto, std::invocable<std::error_code> auto) {}

  void notify(QuantityOf<mp_units::isq::volume> auto, std::invocable<std::error_code> auto) {}

  std::error_code stop() {
    return motor_error(errors::err_enum::motor_not_implemented);
  }

  void stop(QuantityOf<mp_units::isq::time> auto) {}

  void quick_stop() {}

  void run() {}

  void run(SpeedRatio) {}
};
}  // namespace tfc::motor::types