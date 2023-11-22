#include <string>

#include <boost/asio.hpp>

#include <mp-units/format.h>
#include <mp-units/ostream.h>
#include <mp-units/systems/isq/isq.h>
#include <mp-units/systems/si/si.h>
#include <mp-units/chrono.h>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/motors/errors.hpp>
#include <tfc/motors/impl.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace tfc::motor::types {
using mp_units::QuantityOf;
using tfc::confman::observable;
using namespace mp_units::si::unit_symbols;
namespace asio = boost::asio;

class virtual_motor {
private:
  struct config {
    using impl = virtual_motor;
    observable<std::string> name;
    std::optional<mp_units::quantity<mp_units::si::milli<mp_units::si::metre> / mp_units::si::second>> nominal;

    struct glaze {
      using T = config;
      static constexpr auto value = glz::object("name", &T::name, "nominal", &T::nominal);
      static constexpr std::string_view name{ "printing_motor" };
    };

    auto operator==(const config&) const noexcept -> bool = default;
  };

public:
  using config_t = config;

  explicit virtual_motor(asio::io_context& ctx, const config& config) : ctx_(ctx), config_(config), logger_(config.name.value()) {
    logger_.info("virtual_motor c-tor: {}", config.name.value());
    config_.name.observe([this](std::string const& new_v, std::string const& old_v) {
      logger_.warn("Printing motor name switched from: {}, to: {}! takes effect after this short message", old_v, new_v);
      logger_ = logger::logger(new_v);
    });
  }

  ~virtual_motor() {}

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
    std::stringstream ss;
    ss << frequency;
    logger_.info("convey running at {}", ss.str());
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
    auto timer = std::make_shared<asio::steady_timer>(ctx_);
    running_ = true;
    timer->expires_after(mp_units::to_chrono_duration(duration));
    timer->async_wait([this, cb, timer = timer, vel, length, duration](auto ec) {
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
    auto timer = std::make_shared<asio::steady_timer>(ctx_);
    running_ = true;
    timer->expires_after(mp_units::to_chrono_duration(time));
    timer->async_wait([this, cb, timer = timer, vel, time](auto ec) {
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

  // TODO: Where should we get the velocity of this from?
  // or the default frequency
  void convey(QuantityOf<mp_units::isq::length> auto length, std::invocable<std::error_code> auto cb) {
    logger_.trace("convey({});", length);
    if (!config_.nominal) {
      cb(motor_error(errors::err_enum::motor_missing_speed_reference));
    }
  }

  void convey(QuantityOf<mp_units::isq::time> auto time, std::invocable<std::error_code> auto cb) {
    logger_.trace("convey({});", time);
    auto timer = std::make_shared<asio::steady_timer>(ctx_);
    running_ = true;
    timer->expires_after(mp_units::to_chrono_duration(time));
    timer->async_wait([this, cb, timer = timer, time](auto ec) {
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

  auto stop() -> std::error_code {
    running_ = true;
    logger_.trace("stop();");
    return {};
  }

  auto is_running() -> bool {
    return running_;
  }

private:
  asio::io_context& ctx_;
  const config_t& config_;
  tfc::logger::logger logger_;
  bool running_{false};
};
}  // namespace tfc::motor::types
