#pragma once

#include <cstdint>
#include <functional>
#include <system_error>

#include <gmock/gmock.h>
#include <mp-units/systems/si/si.h>
#include <mp-units/quantity.h>

#include <tfc/motor.hpp>

namespace tfc::motor {

namespace detail {

using nanometre_t = mp_units::quantity<mp_units::si::nano<mp_units::si::metre>, std::int64_t>;
using micrometre_t = mp_units::quantity<mp_units::si::micro<mp_units::si::metre>, std::int64_t>;
using millimetre_t = mp_units::quantity<mp_units::si::milli<mp_units::si::metre>, std::int64_t>;
using metre_t = mp_units::quantity<mp_units::si::metre, std::int64_t>;

using nanometre_double_t = mp_units::quantity<mp_units::si::nano<mp_units::si::metre>, std::double_t>;
using micrometre_double_t = mp_units::quantity<mp_units::si::micro<mp_units::si::metre>, std::double_t>;
using millimetre_double_t = mp_units::quantity<mp_units::si::milli<mp_units::si::metre>, std::double_t>;
using metre_double_t = mp_units::quantity<mp_units::si::metre, std::double_t>;

using nanovel_t = mp_units::quantity<nanometre_t::reference / mp_units::si::second, std::int64_t>;
using microvel_t = mp_units::quantity<micrometre_t::reference / mp_units::si::second, std::int64_t>;
using millivel_t = mp_units::quantity<millimetre_t::reference / mp_units::si::second, std::int64_t>;
using vel_t = mp_units::quantity<metre_t::reference / mp_units::si::second, std::int64_t>;

using nanovel_double_t = mp_units::quantity<nanometre_double_t::reference / mp_units::si::second, std::double_t>;
using microvel_double_t = mp_units::quantity<micrometre_double_t::reference / mp_units::si::second, std::double_t>;
using millivel_double_t = mp_units::quantity<millimetre_double_t::reference / mp_units::si::second, std::double_t>;
using vel_double_t = mp_units::quantity<metre_double_t::reference / mp_units::si::second, std::double_t>;

using nanosecond_t = mp_units::quantity<mp_units::si::nano<mp_units::si::second>, std::int64_t>;
using microsecond_t = mp_units::quantity<mp_units::si::micro<mp_units::si::second>, std::int64_t>;
using millisecond_t = mp_units::quantity<mp_units::si::milli<mp_units::si::second>, std::int64_t>;
using second_t = mp_units::quantity<mp_units::si::second, std::int64_t>;

using nanosecond_double_t = mp_units::quantity<mp_units::si::nano<mp_units::si::second>, std::double_t>;
using microsecond_double_t = mp_units::quantity<mp_units::si::micro<mp_units::si::second>, std::double_t>;
using millisecond_double_t = mp_units::quantity<mp_units::si::milli<mp_units::si::second>, std::double_t>;
using second_double_t = mp_units::quantity<mp_units::si::second, std::double_t>;

}  // namespace detail

#define TFC_MOTOR_DEFINE_WITH_TRAVEL_TYPE(velocity_t) \
  X(velocity_t, nanometre_t) \
  X(velocity_t, micrometre_t) \
  X(velocity_t, millimetre_t) \
  X(velocity_t, metre_t) \
  X(velocity_t, nanometre_double_t) \
  X(velocity_t, micrometre_double_t) \
  X(velocity_t, millimetre_double_t) \
  X(velocity_t, metre_double_t)

#define TFC_MOTOR_DEFINE_WITH_TRAVEL_TIME_TYPE(velocity_t, time_t) \
  X(velocity_t, time_t, nanometre_t) \
  X(velocity_t, time_t, micrometre_t) \
  X(velocity_t, time_t, millimetre_t) \
  X(velocity_t, time_t, metre_t) \
  X(velocity_t, time_t, nanometre_double_t) \
  X(velocity_t, time_t, micrometre_double_t) \
  X(velocity_t, time_t, millimetre_double_t) \
  X(velocity_t, time_t, metre_double_t)

#define TFC_MOTOR_DEFINE_WITH_TIME_TYPE(velocity_t) \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TIME_TYPE(velocity_t, nanosecond_t) \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TIME_TYPE(velocity_t, microsecond_t) \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TIME_TYPE(velocity_t, millisecond_t) \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TIME_TYPE(velocity_t, second_t) \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TIME_TYPE(velocity_t, nanosecond_double_t) \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TIME_TYPE(velocity_t, microsecond_double_t) \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TIME_TYPE(velocity_t, millisecond_double_t) \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TIME_TYPE(velocity_t, second_double_t)

#define X_TFC_MOTOR_TRAVEL_TYPES \
  X(nanometre_t) \
  X(micrometre_t) \
  X(millimetre_t) \
  X(metre_t) \
  X(nanometre_double_t) \
  X(micrometre_double_t) \
  X(millimetre_double_t) \
  X(metre_double_t)

#define X_TFC_MOTOR_VELOCITY_TRAVEL_TYPES \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TYPE(nanovel_t) \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TYPE(microvel_t) \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TYPE(millivel_t) \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TYPE(vel_t) \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TYPE(nanovel_double_t) \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TYPE(microvel_double_t) \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TYPE(millivel_double_t) \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TYPE(vel_double_t)

#define X_TFC_MOTOR_VELOCITY_TIME_TRAVEL_TYPES \
  TFC_MOTOR_DEFINE_WITH_TIME_TYPE(nanovel_t) \
  TFC_MOTOR_DEFINE_WITH_TIME_TYPE(microvel_t) \
  TFC_MOTOR_DEFINE_WITH_TIME_TYPE(millivel_t) \
  TFC_MOTOR_DEFINE_WITH_TIME_TYPE(vel_t) \
  TFC_MOTOR_DEFINE_WITH_TIME_TYPE(nanovel_double_t) \
  TFC_MOTOR_DEFINE_WITH_TIME_TYPE(microvel_double_t) \
  TFC_MOTOR_DEFINE_WITH_TIME_TYPE(millivel_double_t) \
  TFC_MOTOR_DEFINE_WITH_TIME_TYPE(vel_double_t)

class mock_api {
public:
  using config_t = typename tfc::motor::api::config_t;
  mock_api(std::shared_ptr<sdbusplus::asio::connection>, std::string_view, config_t = {}) {}

  // Generate 8*8=64 function with name like convey_nanovel_t_nanometre_t
  // etc.
#define X(velocity_t, travel_t) \
  MOCK_METHOD((void), convey_##velocity_t##_##travel_t, (detail::velocity_t, std::function<void(std::error_code, detail::travel_t)>), (const));
  X_TFC_MOTOR_VELOCITY_TRAVEL_TYPES
#undef X

  template <QuantityOf<mp_units::isq::length> travel_t = detail::micrometre_t>
  auto convey(QuantityOf<mp_units::isq::velocity> auto velocity,
            asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type {
#define X(macro_velocity_t, macro_travel_t) \
  if constexpr (std::is_same_v<detail::macro_travel_t, travel_t> && std::is_same_v<detail::macro_velocity_t, std::remove_cvref_t<decltype(velocity)>>) { \
    return convey_##macro_velocity_t##_##macro_travel_t(velocity, std::forward<decltype(token)>(token)); \
  }
  X_TFC_MOTOR_VELOCITY_TRAVEL_TYPES
#undef X
  }

#define X(velocity_t, travel_t) \
  MOCK_METHOD((void), convey_##velocity_t##_##travel_t, (detail::velocity_t, detail::travel_t, std::function<void(std::error_code, detail::travel_t)>), (const));
  X_TFC_MOTOR_VELOCITY_TRAVEL_TYPES
#undef X

   template <QuantityOf<mp_units::isq::length> travel_t>
   auto convey(QuantityOf<mp_units::isq::velocity> auto velocity,
             travel_t length,
             asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
     typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type {
#define X(macro_velocity_t, macro_travel_t) \
    if constexpr (std::is_same_v<detail::macro_travel_t, travel_t> && std::is_same_v<detail::macro_velocity_t, std::remove_cvref_t<decltype(velocity)>>) { \
      return convey_##macro_velocity_t##_##macro_travel_t(velocity, length, std::forward<decltype(token)>(token)); \
    }
    X_TFC_MOTOR_VELOCITY_TRAVEL_TYPES
#undef X
  }

#define X(velocity_t, time_t, travel_t) \
  MOCK_METHOD((void), convey_##velocity_t##_##time_t##_##travel_t, (detail::velocity_t, detail::time_t, std::function<void(std::error_code, detail::travel_t)>), (const));
  X_TFC_MOTOR_VELOCITY_TIME_TRAVEL_TYPES
#undef X

  template <QuantityOf<mp_units::isq::length> travel_t = micrometre_t>
  auto convey(QuantityOf<mp_units::isq::velocity> auto velocity,
            QuantityOf<mp_units::isq::time> auto time,
            asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type {
#define X(macro_velocity_t, macro_time_t, macro_travel_t) \
    if constexpr (std::is_same_v<detail::macro_travel_t, travel_t> && \
        std::is_same_v<detail::macro_velocity_t, std::remove_cvref_t<decltype(velocity)>> && \
        std::is_same_v<detail::macro_time_t, std::remove_cvref_t<decltype(time)>>) { \
      return convey_##macro_velocity_t##_##macro_time_t##_##macro_travel_t(velocity, time, std::forward<decltype(token)>(token)); \
    }
    X_TFC_MOTOR_VELOCITY_TIME_TRAVEL_TYPES
#undef X
  }

#define X(travel_t) \
  MOCK_METHOD((void), convey_##travel_t, (detail::travel_t, std::function<void(std::error_code, detail::travel_t)>), (const));
  X_TFC_MOTOR_TRAVEL_TYPES
#undef X

  template <QuantityOf<mp_units::isq::length> travel_t>
  auto convey(travel_t length, asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type {
#define X(macro_travel_t) \
    if constexpr (std::is_same_v<detail::macro_travel_t, travel_t>) { \
      return convey_##macro_travel_t(length, std::forward<decltype(token)>(token)); \
    }
    X_TFC_MOTOR_TRAVEL_TYPES
#undef X
  }

#define X(position_t) \
  MOCK_METHOD((void), move_##position_t, (speedratio_t, detail::position_t, std::function<void(std::error_code, detail::position_t)>), (const));
  X_TFC_MOTOR_TRAVEL_TYPES
#undef X
/*
  template <QuantityOf<mp_units::isq::length> position_t>
  auto move(speedratio_t speedratio, position_t length, asio::completion_token_for<void(std::error_code, position_t)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, position_t)>::return_type {
    if constexpr (std::is_same_v<void, int>) {
      []<bool flag = false>() {
        static_assert(flag, "This should never be valid");
      }
      ();
    }
#define X(macro_position_t) \
    else if constexpr (std::is_same_v<detail::macro_position_t, position_t>) { \
      return move_##macro_position_t(speedratio, length, std::forward<decltype(token)>(token)); \
    }
    X_TFC_MOTOR_TRAVEL_TYPES
#undef X
    else {
      []<bool flag = false>() {
        static_assert(flag, "The given argument is not implemented, yet. Please open an issue.");
      }
      ();
    }
  }
*/
/*
#define X(position_t) \
  MOCK_METHOD((void), move_##position_t, (detail::position_t, std::function<void(std::error_code, detail::position_t)>), (const));
  X_TFC_MOTOR_TRAVEL_TYPES
#undef X

  template <QuantityOf<mp_units::isq::length> position_t>
  auto move(position_t length, asio::completion_token_for<void(std::error_code, position_t)> auto&& token) ->
    typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, position_t)>::return_type {
    if constexpr (std::is_same_v<void, int>) {
      []<bool flag = false>() {
        static_assert(flag, "This should never be valid");
      }
      ();
    }
#define X(macro_position_t) \
    else if constexpr (std::is_same_v<detail::macro_position_t, position_t>) { \
      return move_##macro_position_t(length, std::forward<decltype(token)>(token)); \
    }
    X_TFC_MOTOR_TRAVEL_TYPES
#undef X
    else {
      []<bool flag = false>() {
        static_assert(flag, "The given argument is not implemented, yet. Please open an issue.");
      }
      ();
    }
  }
*/
};

}  // namespace tfc::motor
