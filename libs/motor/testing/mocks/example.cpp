
#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <tfc/mocks/motor.hpp>

namespace asio = boost::asio;

int main() {
  ::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleMock();

  asio::io_context ctx{};
  auto dbus{ std::make_shared<sdbusplus::asio::connection>(ctx) };

  tfc::motor::mock_api api{ dbus, "foo" };

  using tfc::motor::detail::metre_double_t;
  using tfc::motor::detail::second_double_t;
  using tfc::motor::detail::vel_double_t;

  auto constexpr test_velocity{ 4.2 * vel_double_t::reference };
  auto constexpr test_length{ 1337.0 * metre_double_t::reference };
  auto constexpr test_sec{ 13.37 * second_double_t::reference };
  tfc::motor::speedratio_t constexpr test_speedratio{ 100 * tfc::motor::speedratio_t::reference };

  {
    EXPECT_CALL(api, convey_vel_double_t_metre_double_t(test_velocity, testing::_));
    api.convey<metre_double_t>(test_velocity, [](std::error_code, auto) {});
  }

  {
    EXPECT_CALL(api, convey_vel_double_t_metre_double_t(test_velocity, test_length, testing::_));
    api.convey(test_velocity, test_length, [](std::error_code, auto) {});
  }

  {
    EXPECT_CALL(api, convey_vel_double_t_second_double_t_metre_double_t(test_velocity, test_sec, testing::_));
    api.convey<metre_double_t>(test_velocity, test_sec, [](std::error_code, auto) {});
  }

  {
    EXPECT_CALL(api, convey_metre_double_t(test_length, testing::_));
    api.convey(test_length, [](std::error_code, auto) {});
  }

  {
    EXPECT_CALL(api, move_metre_double_t(test_speedratio, test_length, testing::_));
    api.move(test_speedratio, test_length, [](std::error_code, auto) {});
  }

  {
    EXPECT_CALL(api, move_metre_double_t(test_length, testing::_));
    api.move(test_length, [](std::error_code, auto) {});
  }

  {
    EXPECT_CALL(api, move_home(testing::A<std::function<void(std::error_code)>>()));
    api.move_home([](std::error_code) {});
  }

  // todo segfault
  /*
  {
    co_spawn(ctx, [&]() -> asio::awaitable<void> {
      EXPECT_CALL(api, move_home(testing::A<asio::use_awaitable_t<>>()));
      co_await api.move_home(asio::use_awaitable);
      co_return;
    }, asio::detached);
    ctx.run_for(std::chrono::seconds{ 10 });
  }
  */

  {
    EXPECT_CALL(api, needs_homing(testing::A<std::function<void(std::error_code, bool)>>()));
    api.needs_homing([](std::error_code, bool) {});
  }

  {
    EXPECT_CALL(api, notify_after_metre_double_t(test_length, testing::_));
    api.notify_after(test_length, [](std::error_code, auto) {});
  }

  {
    EXPECT_CALL(api, notify_from_home_metre_double_t(test_length, testing::_));
    api.notify_from_home(test_length, [](std::error_code, auto) {});
  }

  {
    EXPECT_CALL(api, stop(testing::A<std::function<void(std::error_code)>>()));
    api.stop([](std::error_code) {});
  }

  {
    EXPECT_CALL(api, quick_stop(testing::A<std::function<void(std::error_code)>>()));
    api.quick_stop([](std::error_code) {});
  }

  {
    EXPECT_CALL(api, run(testing::A<std::function<void(std::error_code)>>()));
    api.run([](std::error_code) {});
  }

  {
    EXPECT_CALL(api, run(testing::A<std::function<void(std::error_code)>>(), tfc::motor::direction_e::backward));
    api.run([](std::error_code) {}, tfc::motor::direction_e::backward);
  }

  {
    EXPECT_CALL(api, run(test_speedratio, testing::A<std::function<void(std::error_code)>>()));
    api.run(test_speedratio, [](std::error_code) {});
  }

  {
    EXPECT_CALL(api, run_second_double_t(test_speedratio, test_sec, testing::_));
    api.run(test_speedratio, test_sec, [](std::error_code) {});
  }

  {
    EXPECT_CALL(api, run_second_double_t(test_sec, testing::_, tfc::motor::direction_e::backward));
    api.run(
        test_sec, [](std::error_code) {}, tfc::motor::direction_e::backward);
  }

  {
    EXPECT_CALL(api, reset(testing::A<std::function<void(std::error_code)>>()));
    api.reset([](std::error_code) {});
  }

  return EXIT_SUCCESS;
}
