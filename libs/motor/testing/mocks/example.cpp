
#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <tfc/mocks/motor.hpp>

namespace asio = boost::asio;

int main () {
  ::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleMock();

  asio::io_context ctx{};
  auto dbus{ std::make_shared<sdbusplus::asio::connection>(ctx) };


  tfc::motor::mock_api api{dbus, "foo"};

  using tfc::motor::detail::metre_double_t;
  using tfc::motor::detail::vel_double_t;
  using tfc::motor::detail::second_double_t;

  auto constexpr test_velocity{ 4.2 * vel_double_t::reference };
  auto constexpr test_length{ 1337.0 * metre_double_t::reference };
  auto constexpr test_sec{ 13.37 * second_double_t::reference };
  auto constexpr test_speedratio{ 100 * tfc::motor::speedratio_t::reference };

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
/*
  {
    EXPECT_CALL(api, move_metre_double_t(test_length, testing::_));
    api.move(test_length, [](std::error_code, auto) {});
  }
*/
  {
    EXPECT_CALL(api, move_metre_double_t(test_speedratio, test_length, testing::_));
    // api.move(test_speedratio, test_length, [](std::error_code, auto) {});
  }

  return EXIT_SUCCESS;
}

