#include <string>

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

/**
 * @brief mock motor class
 * @details This struct is only used for testing purposes.
 * none of the methods should be called in production.
 * This struct is also simple and does not rely on time by
 * default.
 */

template <typename clock_t>
struct mock_motor {
  explicit virtual_motor(boost::asio::io_context&) {}

  ~virtual_motor() {}

  auto convey() -> std::error_code {}

  auto convey(QuantityOf<mp_units::isq::velocity> auto vel) -> std::error_code {}

  void convey(QuantityOf<mp_units::isq::velocity> auto vel,
              QuantityOf<mp_units::isq::length> auto length,
              std::invocable<std::error_code> auto cb) {}

  void convey(QuantityOf<mp_units::isq::velocity> auto vel,
              QuantityOf<mp_units::isq::time> auto time,
              std::invocable<std::error_code> auto cb) {}

  void convey(QuantityOf<mp_units::isq::length> auto length, std::invocable<std::error_code> auto cb) {}

  void convey(QuantityOf<mp_units::isq::time> auto time, std::invocable<std::error_code> auto) {}
};
bool needs_homing{ false };
bool motor_failure{ false };
std::optional<mp_units::quantity<mp_units::si::milli<mp_units::si::metre> / mp_units::si::second>> nominal{
  10 * (mp_units::si::milli<mp_units::si::metre> / mp_units::si::second)
};
bool in_action{ false };
}  // namespace tfc::motor::types
