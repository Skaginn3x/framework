#include <string>

#include <mp-units/chrono.h>
#include <mp-units/format.h>
#include <mp-units/math.h>
#include <mp-units/ostream.h>
#include <mp-units/systems/isq/isq.h>
#include <mp-units/systems/si/si.h>
#include <boost/asio.hpp>

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
namespace chrono = std::chrono;

/**
 * @brief Virtual motor class
 * @details This class is used to simulate a motor, it does not actually control a motor.
 * for use in testing use mock_motor in mock_motor.hpp. That motor can not be selected
 * as a output of a running program and exposes a greater deal of internals.
 */

class virtual_motor {
private:
  struct config {
    using impl = virtual_motor;
    observable<std::string> name;
    std::optional<mp_units::quantity<mp_units::si::milli<mp_units::si::metre> / mp_units::si::second>> nominal;

    struct glaze {
      using T = config;
      static constexpr auto value = glz::object("name",
                                                &T::name,
                                                "Name of the motor",
                                                "nominal",
                                                &T::nominal,
                                                "Speed of virtual motor in physical quantities while at 50Hz");
      static constexpr std::string_view name{ "printing_motor" };
    };

    auto operator==(const config&) const noexcept -> bool = default;
  };

public:
  using config_t = config;

  explicit virtual_motor(asio::io_context& ctx, const config& config)
      : ctx_(ctx), config_(config), logger_(config.name.value()), timer_{ ctx_ } {
    logger_.info("virtual_motor c-tor: {}", config.name.value());
    config_.name.observe([this](std::string const& new_v, std::string const& old_v) {
      logger_.warn("Printing motor name switched from: {}, to: {}! takes effect after this short message", old_v, new_v);
      logger_ = logger::logger(new_v);
    });
  }

  auto convey() -> std::error_code {
    logger_.info("convey!");
    running_ = true;
    return {};
  }

  auto convey(QuantityOf<mp_units::isq::velocity> auto vel) -> std::error_code {
    logger_.trace("convey({});", vel);
    if (!config_.nominal) {
      return motor_error(errors::err_enum::motor_missing_speed_reference);
    }
    [[maybe_unused]] auto frequency = tfc::motor::impl::nominal_at_50Hz_to_frequency(config_.nominal.value(), vel);
    logger_.info("convey running at {}", frequency);
    running_ = true;
    return {};
  }

  void convey(QuantityOf<mp_units::isq::velocity> auto vel,
              QuantityOf<mp_units::isq::length> auto length,
              std::invocable<std::error_code> auto cb) {
    logger_.trace("convey({}, {});", vel, length);
    if (!config_.nominal) {
      cb(motor_error(errors::err_enum::motor_missing_speed_reference));
    }
    auto duration = length / vel;
    running_ = true;
    // Someone could already be moving or running the motor.
    timer_.cancel();
    timer_.expires_after(chrono::duration_cast<chrono::nanoseconds>(mp_units::to_chrono_duration(duration)));
    timer_.async_wait([this, cb, vel, length, duration](auto ec) {
      if (ec) {
        logger_.trace("convey({}, {}); canceled", vel, length);
        cb(ec);
        return;
      }
      running_ = false;
      logger_.trace("convey({}, {}); finished in {}!", vel, length, duration);
      cb({});
    });
  }

  void convey(QuantityOf<mp_units::isq::velocity> auto vel,
              QuantityOf<mp_units::isq::time> auto time,
              std::invocable<std::error_code> auto cb) {
    logger_.trace("convey({}, {});", vel, time);
    if (!config_.nominal) {
      cb(motor_error(errors::err_enum::motor_missing_speed_reference));
    }
    running_ = true;
    timer_.cancel();
    timer_.expires_after(mp_units::to_chrono_duration(time));
    timer_.async_wait([this, cb, vel, time](auto ec) {
      if (ec) {
        logger_.trace("convey({}, {}); canceled", vel, time);
        cb(ec);
        return;
      }
      running_ = false;
      auto length = vel * time;
      logger_.trace("convey({}, {}); ran {}!", vel, time, length);
      cb({});
    });
  }

  template <QuantityOf<mp_units::isq::length> travel_t>
  void convey(travel_t length, std::invocable<std::error_code, travel_t> auto cb) {
    logger_.trace("convey({});", length);
    if (!config_.nominal) {
      cb(motor_error(errors::err_enum::motor_missing_speed_reference), 0 * travel_t::reference);
    }
  }

  void convey(QuantityOf<mp_units::isq::time> auto time, std::invocable<std::error_code> auto cb) {
    logger_.trace("convey({});", time);
    // TODO: Implement
    running_ = true;
    timer_.cancel();
    timer_.expires_after(chrono::duration_cast<chrono::nanoseconds>(mp_units::to_chrono_duration(time)));
    timer_.async_wait([this, cb, time](auto ec) {
      if (ec) {
        logger_.trace("convey({}); canceled", time);
        cb(ec);
        return;
      }
      running_ = false;
      logger_.trace("convey({}); ran!", time);
      cb({});
    });
  }

  void move(QuantityOf<mp_units::isq::length> auto position, std::invocable<std::error_code> auto cb) {
    logger_.trace("move({});", position);
    if (!config_.nominal) {
      cb(motor_error(errors::err_enum::motor_missing_speed_reference));
      return;
    }
    auto length = mp_units::abs(pos_ - position);
    auto duration = length / config_.nominal.value();
    timer_.cancel();
    timer_.expires_after(chrono::duration_cast<chrono::nanoseconds>(mp_units::to_chrono_duration(duration)));
    timer_.async_wait([this, cb, position](auto ec) {
      if (ec) {
        logger_.trace("move({}); canceled", position);
        cb(ec);
        return;
      }
      running_ = false;
      pos_ = position;
      logger_.trace("move({}); complete!", position);
      cb({});
    });
  }

  void move_home(std::invocable<std::error_code> auto cb) {
    logger_.trace("move_home();");
    timer_.cancel();
    timer_.expires_after(chrono::seconds(1));
    timer_.async_wait([&, cb](const std::error_code& err) {
      if (err) {
        logger_.trace("move_home(); canceled");
        cb(err);
        return;
      }
      has_reference_ = true;
      logger_.trace("move_home(); complete!");
      cb({});
    });
  }

  auto needs_homing() const -> std::expected<bool, std::error_code> {
    logger_.trace("needs_homing();");
    return !has_reference_;
  }

  auto stop() -> std::error_code {
    running_ = false;
    logger_.trace("stop();");
    return {};
  }

  auto is_running() const -> bool { return running_; }

private:
  asio::io_context& ctx_;
  const config_t& config_;
  tfc::logger::logger logger_;
  asio::steady_timer timer_;
  mp_units::quantity<mp_units::si::milli<mp_units::si::metre>> pos_{ 0 * mm };
  bool has_reference_{ false };
  bool running_{ false };
};
}  // namespace tfc::motor::types
