#include <string>

#include <mp-units/format.h>
#include <mp-units/ostream.h>
#include <mp-units/systems/isq/isq.h>
#include <mp-units/systems/si/si.h>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/motors/errors.hpp>
#include <tfc/motors/impl.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace tfc::motor::types {
using mp_units::QuantityOf;
using tfc::confman::observable;
using namespace mp_units::si::unit_symbols;


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
      static constexpr auto value = glz::object("name", &T::name, "nominal", &T::nominal);
      static constexpr std::string_view name{ "printing_motor" };
    };

    auto operator==(const config&) const noexcept -> bool = default;
  };

public:
  using config_t = config;

  explicit virtual_motor(boost::asio::io_context&, const config& config) : config_(config), logger_(config.name.value()) {
    logger_.info("virtual_motor c-tor: {}", config.name.value());
    config_.name.observe([this](std::string const& new_v, std::string const& old_v) {
      logger_.warn("Printing motor name switched from: {}, to: {}! takes effect after this short message", old_v, new_v);
      logger_ = logger::logger(new_v);
    });
  }

  ~virtual_motor() {}

  auto convey() -> std::error_code {
    logger_.info("convey!");
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
    return {};
  }

  void convey(QuantityOf<mp_units::isq::velocity> auto vel,
              QuantityOf<mp_units::isq::length> auto length,
              std::invocable<std::error_code> auto cb) {
    logger_.trace("convey({}, {});", vel, length);
    if (!config_.nominal) {
      cb(motor_error(errors::err_enum::motor_missing_speed_reference));
    }
  }

  void convey(QuantityOf<mp_units::isq::velocity> auto vel,
              QuantityOf<mp_units::isq::time> auto time,
              std::invocable<std::error_code> auto cb) {
    logger_.trace("convey({}, {});", vel, time);
    if (!config_.nominal) {
      cb(motor_error(errors::err_enum::motor_missing_speed_reference));
    }
  }

  void convey(QuantityOf<mp_units::isq::length> auto length, std::invocable<std::error_code> auto cb) {
    logger_.trace("convey({});", length);
    if (!config_.nominal) {
      cb(motor_error(errors::err_enum::motor_missing_speed_reference));
    }
  }

  void convey(QuantityOf<mp_units::isq::time> auto time, std::invocable<std::error_code> auto) {
    logger_.trace("convey({});", time);
  }

private:
  const config_t& config_;
  tfc::logger::logger logger_;
};
}  // namespace tfc::motor::types
