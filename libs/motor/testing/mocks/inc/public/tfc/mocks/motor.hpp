#pragma once

#include <cstdint>
#include <functional>
#include <system_error>

#include <gmock/gmock.h>
#include <mp-units/quantity.h>
#include <mp-units/systems/si/si.h>

#include <tfc/motor.hpp>

namespace tfc::motor {

namespace asio = boost::asio;

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
  X(velocity_t, nanometre_t)                          \
  X(velocity_t, micrometre_t)                         \
  X(velocity_t, millimetre_t)                         \
  X(velocity_t, metre_t)                              \
  X(velocity_t, nanometre_double_t)                   \
  X(velocity_t, micrometre_double_t)                  \
  X(velocity_t, millimetre_double_t)                  \
  X(velocity_t, metre_double_t)

#define TFC_MOTOR_DEFINE_WITH_TRAVEL_TIME_TYPE(velocity_t, time_t) \
  X(velocity_t, time_t, nanometre_t)                               \
  X(velocity_t, time_t, micrometre_t)                              \
  X(velocity_t, time_t, millimetre_t)                              \
  X(velocity_t, time_t, metre_t)                                   \
  X(velocity_t, time_t, nanometre_double_t)                        \
  X(velocity_t, time_t, micrometre_double_t)                       \
  X(velocity_t, time_t, millimetre_double_t)                       \
  X(velocity_t, time_t, metre_double_t)

#define TFC_MOTOR_DEFINE_WITH_TIME_TYPE(velocity_t)                        \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TIME_TYPE(velocity_t, nanosecond_t)         \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TIME_TYPE(velocity_t, microsecond_t)        \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TIME_TYPE(velocity_t, millisecond_t)        \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TIME_TYPE(velocity_t, second_t)             \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TIME_TYPE(velocity_t, nanosecond_double_t)  \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TIME_TYPE(velocity_t, microsecond_double_t) \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TIME_TYPE(velocity_t, millisecond_double_t) \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TIME_TYPE(velocity_t, second_double_t)

#define X_TFC_MOTOR_TRAVEL_TYPES \
  X(nanometre_t)                 \
  X(micrometre_t)                \
  X(millimetre_t)                \
  X(metre_t)                     \
  X(nanometre_double_t)          \
  X(micrometre_double_t)         \
  X(millimetre_double_t)         \
  X(metre_double_t)

#define X_TFC_MOTOR_TIME_TYPES \
  X(nanosecond_t)              \
  X(microsecond_t)             \
  X(millisecond_t)             \
  X(second_t)                  \
  X(nanosecond_double_t)       \
  X(microsecond_double_t)      \
  X(millisecond_double_t)      \
  X(second_double_t)

#define X_TFC_MOTOR_VELOCITY_TRAVEL_TYPES              \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TYPE(nanovel_t)         \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TYPE(microvel_t)        \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TYPE(millivel_t)        \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TYPE(vel_t)             \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TYPE(nanovel_double_t)  \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TYPE(microvel_double_t) \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TYPE(millivel_double_t) \
  TFC_MOTOR_DEFINE_WITH_TRAVEL_TYPE(vel_double_t)

#define X_TFC_MOTOR_VELOCITY_TIME_TRAVEL_TYPES       \
  TFC_MOTOR_DEFINE_WITH_TIME_TYPE(nanovel_t)         \
  TFC_MOTOR_DEFINE_WITH_TIME_TYPE(microvel_t)        \
  TFC_MOTOR_DEFINE_WITH_TIME_TYPE(millivel_t)        \
  TFC_MOTOR_DEFINE_WITH_TIME_TYPE(vel_t)             \
  TFC_MOTOR_DEFINE_WITH_TIME_TYPE(nanovel_double_t)  \
  TFC_MOTOR_DEFINE_WITH_TIME_TYPE(microvel_double_t) \
  TFC_MOTOR_DEFINE_WITH_TIME_TYPE(millivel_double_t) \
  TFC_MOTOR_DEFINE_WITH_TIME_TYPE(vel_double_t)

/// \brief GMock API for tfc::motor::api.
/// \example libs/motor/testing/mocks/example.cpp
/// Please refer to the typedefs in namespace detail above, they do make the names of the mocking functions.
/// NOTE: You need to match the types exactly.
/// Often you can deduce the actual function name from GMock non interesting function call Warning message.
/// \note all functions are const even though the actual API is not.
/// \todo add capability to mock awaitable supporting co_await
class mock_api {
public:
  using config_t = typename tfc::motor::api::config_t;
  mock_api(std::shared_ptr<sdbusplus::asio::connection>, std::string_view, config_t = {}) {}

  /// \brief Convey at the given velocity until interrupted.
  /// Mock Signature: convey_<velocity_t>_<travel_t>(velocity_t, travel_t, std::function<void(std::error_code, travel_t)>)
  /// \note please refer to substitute "<>" naming in typedefs in detail namespace above
  /// \code
  ///   tfc::motor::mock_api api{ dbus, "foo" };
  ///   auto constexpr test_velocity{ 4.2 * detail::vel_double_t::reference };
  ///   EXPECT_CALL(api, convey_vel_double_t_metre_double_t(test_velocity, testing::_));
  ///   api.convey<metre_double_t>(test_velocity, [](std::error_code, auto) {});
  /// \endcode
  template <QuantityOf<mp_units::isq::length> travel_t = detail::micrometre_t>
  auto convey(QuantityOf<mp_units::isq::velocity> auto velocity,
              asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type {
    // clang-format off
    if constexpr (std::is_same_v<void, int>) { []<bool flag = false>() { static_assert(flag, "This should never be valid"); }(); }
    // clang-format on
#define X(macro_velocity_t, macro_travel_t)                                                               \
  else if constexpr (std::is_same_v<detail::macro_travel_t, travel_t> &&                                  \
                     std::is_same_v<detail::macro_velocity_t, std::remove_cvref_t<decltype(velocity)>>) { \
    return convey_##macro_velocity_t##_##macro_travel_t(velocity, std::forward<decltype(token)>(token));  \
  }
    X_TFC_MOTOR_VELOCITY_TRAVEL_TYPES
#undef X
    // clang-format off
    else { []<bool flag = false>() { static_assert(flag, "The given argument is not implemented, yet. Please open an issue."); }(); }
    // clang-format on
  }

  /// \brief Convey at the given velocity until reached the given length.
  /// Mock Signature: convey_<velocity_t>_<travel_t>(velocity_t, travel_t, std::function<void(std::error_code, travel_t)>)
  /// \note please refer to substitute "<>" naming in typedefs in detail namespace above
  /// \code
  ///   tfc::motor::mock_api api{ dbus, "foo" };
  ///   auto constexpr test_velocity{ 4.2 * detail::vel_double_t::reference };
  ///   auto constexpr test_length{ 1337.0 * detail::metre_double_t::reference };
  ///   EXPECT_CALL(api, convey_vel_double_t_metre_double_t(test_velocity, test_length, testing::_));
  ///   api.convey(test_velocity, test_length, [](std::error_code, auto) {});
  /// \endcode
  template <QuantityOf<mp_units::isq::length> travel_t>
  auto convey(QuantityOf<mp_units::isq::velocity> auto velocity,
              travel_t length,
              asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type {
    // clang-format off
    if constexpr (std::is_same_v<void, int>) { []<bool flag = false>() { static_assert(flag, "This should never be valid"); }(); }
    // clang-format on
#define X(macro_velocity_t, macro_travel_t)                                                                      \
  else if constexpr (std::is_same_v<detail::macro_travel_t, travel_t> &&                                         \
                     std::is_same_v<detail::macro_velocity_t, std::remove_cvref_t<decltype(velocity)>>) {        \
    return convey_##macro_velocity_t##_##macro_travel_t(velocity, length, std::forward<decltype(token)>(token)); \
  }
    X_TFC_MOTOR_VELOCITY_TRAVEL_TYPES
#undef X
    // clang-format off
    else { []<bool flag = false>() { static_assert(flag, "The given argument is not implemented, yet. Please open an issue."); }(); }
    // clang-format on
  }

  /// \brief Convey at the given velocity until reached the given time.
  /// Mock Signature: convey_<velocity_t>_<time_t>_<travel_t>(velocity_t, time_t, std::function<void(std::error_code,
  /// travel_t)>)
  /// \note please refer to substitute "<>" naming in typedefs in detail namespace above
  /// \code
  ///   tfc::motor::mock_api api{ dbus, "foo" };
  ///   auto constexpr test_velocity{ 4.2 * detail::vel_double_t::reference };
  ///   auto constexpr test_sec{ 13.37 * detail::second_double_t::reference };
  ///   EXPECT_CALL(api, convey_vel_double_t_second_double_t_metre_double_t(test_velocity, test_sec, testing::_));
  ///   api.convey<metre_double_t>(test_velocity, test_sec, [](std::error_code, auto) {});
  /// \endcode
  template <QuantityOf<mp_units::isq::length> travel_t = micrometre_t>
  auto convey(QuantityOf<mp_units::isq::velocity> auto velocity,
              QuantityOf<mp_units::isq::time> auto time,
              asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type {
    // clang-format off
    if constexpr (std::is_same_v<void, int>) { []<bool flag = false>() { static_assert(flag, "This should never be valid"); }(); }
    // clang-format on
#define X(macro_velocity_t, macro_time_t, macro_travel_t)                                                       \
  else if constexpr (std::is_same_v<detail::macro_travel_t, travel_t> &&                                        \
                     std::is_same_v<detail::macro_velocity_t, std::remove_cvref_t<decltype(velocity)>> &&       \
                     std::is_same_v<detail::macro_time_t, std::remove_cvref_t<decltype(time)>>) {               \
    return convey_##macro_velocity_t##_##macro_time_t##_##macro_travel_t(velocity, time,                        \
                                                                         std::forward<decltype(token)>(token)); \
  }
    X_TFC_MOTOR_VELOCITY_TIME_TRAVEL_TYPES
#undef X
    // clang-format off
    else { []<bool flag = false>() { static_assert(flag, "The given argument is not implemented, yet. Please open an issue."); }(); }
    // clang-format on
  }

  /// \brief Convey at the default velocity until reached the given length.
  /// Mock Signature: convey_<travel_t>(travel_t, std::function<void(std::error_code, travel_t)>)
  /// \note please refer to substitute "<>" naming in typedefs in detail namespace above
  /// \code
  ///   tfc::motor::mock_api api{ dbus, "foo" };
  ///   auto constexpr test_length{ 1337.0 * detail::metre_double_t::reference };
  ///   EXPECT_CALL(api, convey_metre_double_t(test_length, testing::_));
  ///   api.convey(test_length, [](std::error_code, auto) {});
  /// \endcode
  template <QuantityOf<mp_units::isq::length> travel_t>
  auto convey(travel_t length, asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type {
    // clang-format off
    if constexpr (std::is_same_v<void, int>) { []<bool flag = false>() { static_assert(flag, "This should never be valid"); }(); }
    // clang-format on
#define X(macro_travel_t)                                                         \
  else if constexpr (std::is_same_v<detail::macro_travel_t, travel_t>) {          \
    return convey_##macro_travel_t(length, std::forward<decltype(token)>(token)); \
  }
    X_TFC_MOTOR_TRAVEL_TYPES
#undef X
    // clang-format off
    else { []<bool flag = false>() { static_assert(flag, "The given argument is not implemented, yet. Please open an issue."); }(); }
    // clang-format on
  }

  /// \brief Move at the given speedratio until given point is reached, can move forward/backward given current position.
  /// Mock Signature: mock_<position_t>(position_t, std::function<void(std::error_code, position_t)>)
  /// \note please refer to substitute "<>" naming in typedefs in detail namespace above
  /// \code
  ///   tfc::motor::mock_api api{ dbus, "foo" };
  ///   auto constexpr test_length{ 1337.0 * detail::metre_double_t::reference };
  ///   tfc::motor::speedratio_t constexpr test_speedratio{ 100 * tfc::motor::speedratio_t::reference };
  ///   EXPECT_CALL(api, move_metre_double_t(test_speedratio, test_length, testing::_));
  ///   api.move(test_speedratio, test_length, [](std::error_code, auto) {});
  /// \endcode
  template <QuantityOf<mp_units::isq::length> position_t>
  auto move(speedratio_t speedratio,
            position_t length,
            asio::completion_token_for<void(std::error_code, position_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, position_t)>::return_type {
    // clang-format off
    if constexpr (std::is_same_v<void, int>) { []<bool flag = false>() { static_assert(flag, "This should never be valid"); }(); }
    // clang-format on
#define X(macro_position_t)                                                                   \
  else if constexpr (std::is_same_v<detail::macro_position_t, position_t>) {                  \
    return move_##macro_position_t(speedratio, length, std::forward<decltype(token)>(token)); \
  }
    X_TFC_MOTOR_TRAVEL_TYPES
#undef X
    // clang-format off
    else { []<bool flag = false>() { static_assert(flag, "The given argument is not implemented, yet. Please open an issue."); }(); }
    // clang-format on
  }

  /// \brief Move at the default speedratio until given point is reached, can move forward/backward given current position.
  /// Mock Signature: mock_<position_t>(position_t, std::function<void(std::error_code, position_t)>)
  /// \note please refer to substitute "<>" naming in typedefs in detail namespace above
  /// \code
  ///   tfc::motor::mock_api api{ dbus, "foo" };
  ///   auto constexpr test_length{ 1337.0 * detail::metre_double_t::reference };
  ///   tfc::motor::speedratio_t constexpr test_speedratio{ 100 * tfc::motor::speedratio_t::reference };
  ///   EXPECT_CALL(api, move_metre_double_t(test_speedratio, test_length, testing::_));
  ///   api.move(test_speedratio, test_length, [](std::error_code, auto) {});
  /// \endcode
  template <QuantityOf<mp_units::isq::length> position_t>
  auto move(position_t length, asio::completion_token_for<void(std::error_code, position_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, position_t)>::return_type {
    // clang-format off
    if constexpr (std::is_same_v<void, int>) { []<bool flag = false>() { static_assert(flag, "This should never be valid"); }(); }
    // clang-format on
#define X(macro_position_t)                                                       \
  else if constexpr (std::is_same_v<detail::macro_position_t, position_t>) {      \
    return move_##macro_position_t(length, std::forward<decltype(token)>(token)); \
  }
    X_TFC_MOTOR_TRAVEL_TYPES
#undef X
    // clang-format off
    else { []<bool flag = false>() { static_assert(flag, "The given argument is not implemented, yet. Please open an issue."); }(); }
    // clang-format on
  }

  /// \brief Move to the home position.
  /// \code
  ///   tfc::motor::mock_api api{ dbus, "foo" };
  ///   EXPECT_CALL(api, move_home(testing::A<std::function<void(std::error_code)>>()));
  ///   api.move_home([](std::error_code) {});
  /// \endcode
  MOCK_METHOD((void), move_home, (std::function<void(std::error_code)>), (const));
  // todo segfault
  // using awaitable_return_t = typename asio::async_result<std::decay_t<asio::use_awaitable_t<>>,
  // void(std::error_code)>::return_type; MOCK_METHOD((awaitable_return_t), move_home, (asio::use_awaitable_t<>), (const));

  /// \brief Check if the motor needs homing.
  /// \code
  ///   tfc::motor::mock_api api{ dbus, "foo" };
  ///   EXPECT_CALL(api, needs_homing(testing::A<std::function<void(std::error_code, bool)>>()));
  ///   api.needs_homing([](std::error_code, bool) {});
  /// \endcode
  MOCK_METHOD((void), needs_homing, (std::function<void(std::error_code, bool)>), (const));

  /// \brief Notify after the given length.
  /// Mock Signature: notify_after_<travel_t>(travel_t, std::function<void(std::error_code, travel_t)>)
  /// \note please refer to substitute "<>" naming in typedefs in detail namespace above
  /// \code
  ///   tfc::motor::mock_api api{ dbus, "foo" };
  ///   auto constexpr test_length{ 1337.0 * detail::metre_double_t::reference };
  ///   EXPECT_CALL(api, notify_after_metre_double_t(test_length, testing::_));
  ///   api.notify_after(test_length, [](std::error_code, auto) {});
  /// \endcode
  template <QuantityOf<mp_units::isq::length> travel_t>
  auto notify_after(travel_t length, asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type {
    // clang-format off
    if constexpr (std::is_same_v<void, int>) { []<bool flag = false>() { static_assert(flag, "This should never be valid"); }(); }
    // clang-format on
#define X(macro_travel_t)                                                               \
  else if constexpr (std::is_same_v<detail::macro_travel_t, travel_t>) {                \
    return notify_after_##macro_travel_t(length, std::forward<decltype(token)>(token)); \
  }
    X_TFC_MOTOR_TRAVEL_TYPES
#undef X
    // clang-format off
    else { []<bool flag = false>() { static_assert(flag, "The given argument is not implemented, yet. Please open an issue."); }(); }
    // clang-format on
  }

  /// \brief Notify after the given length.
  /// Mock Signature: notify_after_<travel_t>(travel_t, std::function<void(std::error_code, travel_t)>)
  /// \note please refer to substitute "<>" naming in typedefs in detail namespace above
  /// \code
  ///   tfc::motor::mock_api api{ dbus, "foo" };
  ///   auto constexpr test_length{ 1337.0 * detail::metre_double_t::reference };
  ///   EXPECT_CALL(api, notify_from_home_metre_double_t(test_length, testing::_));
  ///   api.notify_from_home(test_length, [](std::error_code, auto) {});
  /// \endcode
  template <QuantityOf<mp_units::isq::length> travel_t>
  auto notify_from_home(travel_t length, asio::completion_token_for<void(std::error_code, travel_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, travel_t)>::return_type {
    // clang-format off
    if constexpr (std::is_same_v<void, int>) { []<bool flag = false>() { static_assert(flag, "This should never be valid"); }(); }
    // clang-format on
#define X(macro_travel_t)                                                                   \
  else if constexpr (std::is_same_v<detail::macro_travel_t, travel_t>) {                    \
    return notify_from_home_##macro_travel_t(length, std::forward<decltype(token)>(token)); \
  }
    X_TFC_MOTOR_TRAVEL_TYPES
#undef X
    // clang-format off
    else { []<bool flag = false>() { static_assert(flag, "The given argument is not implemented, yet. Please open an issue."); }(); }
    // clang-format on
  }

  /// \brief Stop Motor.
  /// \code
  ///   tfc::motor::mock_api api{ dbus, "foo" };
  ///   EXPECT_CALL(api, stop(testing::A<std::function<void(std::error_code)>>()));
  ///   api.stop([](std::error_code) {});
  /// \endcode
  MOCK_METHOD((void), stop, (std::function<void(std::error_code)>), (const));

  /// \brief Quick Stop Motor.
  /// \code
  ///   tfc::motor::mock_api api{ dbus, "foo" };
  ///   EXPECT_CALL(api, quick_stop(testing::A<std::function<void(std::error_code)>>()));
  ///   api.quick_stop([](std::error_code) {});
  /// \endcode
  MOCK_METHOD((void), quick_stop, (std::function<void(std::error_code)>), (const));

  /// \brief Run Motor.
  /// \code
  ///   tfc::motor::mock_api api{ dbus, "foo" };
  ///   EXPECT_CALL(api, run(testing::A<std::function<void(std::error_code)>>()));
  ///   api.run([](std::error_code) {});
  /// \endcode
  MOCK_METHOD((void), run, (std::function<void(std::error_code)>), (const));

  /// \brief Run Motor in specific direction.
  /// \code
  ///   tfc::motor::mock_api api{ dbus, "foo" };
  ///   EXPECT_CALL(api, run(testing::A<std::function<void(std::error_code)>>(), tfc::motor::direction_e::backward));
  ///   api.run([](std::error_code) {}, tfc::motor::direction_e::backward);
  /// \endcode
  MOCK_METHOD((void), run, (std::function<void(std::error_code)>, direction_e), (const));

  /// \brief Run Motor at specific speedratio.
  /// \code
  ///   tfc::motor::mock_api api{ dbus, "foo" };
  ///   tfc::motor::speedratio_t constexpr test_speedratio{ 100 * tfc::motor::speedratio_t::reference };
  ///   EXPECT_CALL(api, run(test_speedratio, testing::A<std::function<void(std::error_code)>>()));
  ///   api.run(test_speedratio, [](std::error_code) {});
  /// \endcode
  MOCK_METHOD((void), run, (speedratio_t, std::function<void(std::error_code)>), (const));

  /// \brief Run motor at the given speedratio for the given time.
  /// Mock Signature: run_<time_t>(time_t, std::function<void(std::error_code)>)
  /// \note please refer to substitute "<>" naming in typedefs in detail namespace above
  /// \code
  ///   tfc::motor::mock_api api{ dbus, "foo" };
  ///   auto constexpr test_sec{ 13.37 * detail::second_double_t::reference };
  ///   tfc::motor::speedratio_t constexpr test_speedratio{ 100 * tfc::motor::speedratio_t::reference };
  ///   EXPECT_CALL(api, run_second_double_t(test_speedratio, test_sec, testing::_));
  ///   api.run(test_speedratio, test_sec, [](std::error_code, auto) {});
  /// \endcode
  template <QuantityOf<mp_units::isq::time> time_t>
  auto run(speedratio_t speedratio, time_t time, asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    // clang-format off
    if constexpr (std::is_same_v<void, int>) { []<bool flag = false>() { static_assert(flag, "This should never be valid"); }(); }
    // clang-format on
#define X(macro_time_t)                                                                \
  else if constexpr (std::is_same_v<detail::macro_time_t, time_t>) {                   \
    return run_##macro_time_t(speedratio, time, std::forward<decltype(token)>(token)); \
  }
    X_TFC_MOTOR_TIME_TYPES
#undef X
    // clang-format off
    else { []<bool flag = false>() { static_assert(flag, "The given argument is not implemented, yet. Please open an issue."); }(); }
    // clang-format on
  }

  /// \brief Run motor at the default speedratio for the given time with the given direction.
  /// Mock Signature: run_<time_t>(time_t, std::function<void(std::error_code)>)
  /// \note please refer to substitute "<>" naming in typedefs in detail namespace above
  /// \code
  ///   tfc::motor::mock_api api{ dbus, "foo" };
  ///   auto constexpr test_sec{ 13.37 * detail::second_double_t::reference };
  ///   EXPECT_CALL(api, run_second_double_t(test_sec, testing::_, tfc::motor::direction_e::backward));
  ///   api.run(test_sec, [](std::error_code) {}, tfc::motor::direction_e::backward);
  /// \endcode
  template <QuantityOf<mp_units::isq::time> time_t>
  auto run(time_t time,
           asio::completion_token_for<void(std::error_code)> auto&& token,
           direction_e direction = direction_e::forward) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    // clang-format off
    if constexpr (std::is_same_v<void, int>) { []<bool flag = false>() { static_assert(flag, "This should never be valid"); }(); }
    // clang-format on
#define X(macro_time_t)                                                               \
  else if constexpr (std::is_same_v<detail::macro_time_t, time_t>) {                  \
    return run_##macro_time_t(time, std::forward<decltype(token)>(token), direction); \
  }
    X_TFC_MOTOR_TIME_TYPES
#undef X
    // clang-format off
    else { []<bool flag = false>() { static_assert(flag, "The given argument is not implemented, yet. Please open an issue."); }(); }
    // clang-format on
  }

  /// \brief Reset any error on Motor.
  /// \code
  ///   tfc::motor::mock_api api{ dbus, "foo" };
  ///   EXPECT_CALL(api, reset(testing::A<std::function<void(std::error_code)>>()));
  ///   api.reset([](std::error_code) {});
  /// \endcode
  MOCK_METHOD((void), reset, (std::function<void(std::error_code)>), (const));

#define X(velocity_t, travel_t)                         \
  MOCK_METHOD((void), convey_##velocity_t##_##travel_t, \
              (detail::velocity_t, std::function<void(std::error_code, detail::travel_t)>), (const));
  X_TFC_MOTOR_VELOCITY_TRAVEL_TYPES
#undef X
#define X(velocity_t, travel_t)                         \
  MOCK_METHOD((void), convey_##velocity_t##_##travel_t, \
              (detail::velocity_t, detail::travel_t, std::function<void(std::error_code, detail::travel_t)>), (const));
  X_TFC_MOTOR_VELOCITY_TRAVEL_TYPES
#undef X
#define X(velocity_t, time_t, travel_t)                            \
  MOCK_METHOD((void), convey_##velocity_t##_##time_t##_##travel_t, \
              (detail::velocity_t, detail::time_t, std::function<void(std::error_code, detail::travel_t)>), (const));
  X_TFC_MOTOR_VELOCITY_TIME_TRAVEL_TYPES
#undef X
#define X(travel_t)                                                                                                  \
  MOCK_METHOD((void), convey_##travel_t, (detail::travel_t, std::function<void(std::error_code, detail::travel_t)>), \
              (const));
  X_TFC_MOTOR_TRAVEL_TYPES
#undef X
#define X(position_t)                    \
  MOCK_METHOD((void), move_##position_t, \
              (speedratio_t, detail::position_t, std::function<void(std::error_code, detail::position_t)>), (const));
  X_TFC_MOTOR_TRAVEL_TYPES
#undef X
#define X(position_t)                                                                                                    \
  MOCK_METHOD((void), move_##position_t, (detail::position_t, std::function<void(std::error_code, detail::position_t)>), \
              (const));
  X_TFC_MOTOR_TRAVEL_TYPES
#undef X
#define X(travel_t)                                                                                                        \
  MOCK_METHOD((void), notify_after_##travel_t, (detail::travel_t, std::function<void(std::error_code, detail::travel_t)>), \
              (const));
  X_TFC_MOTOR_TRAVEL_TYPES
#undef X
#define X(travel_t)                                \
  MOCK_METHOD((void), notify_from_home_##travel_t, \
              (detail::travel_t, std::function<void(std::error_code, detail::travel_t)>), (const));
  X_TFC_MOTOR_TRAVEL_TYPES
#undef X
#define X(time_t) \
  MOCK_METHOD((void), run_##time_t, (speedratio_t, detail::time_t, std::function<void(std::error_code)>), (const));
  X_TFC_MOTOR_TIME_TYPES
#undef X
#define X(time_t) \
  MOCK_METHOD((void), run_##time_t, (detail::time_t, std::function<void(std::error_code)>, direction_e), (const));
  X_TFC_MOTOR_TIME_TYPES
#undef X
};

}  // namespace tfc::motor
