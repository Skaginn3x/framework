#include <boost/asio.hpp>
#include <boost/ut.hpp>
#include <sdbusplus/asio/connection.hpp>

#include <tfc/mocks/confman.hpp>
#include <tfc/mocks/confman/file_storage.hpp>
#include <tfc/progbase.hpp>
#include <tfc/dbus/sd_bus.hpp>

namespace asio = boost::asio;
namespace ut = boost::ut;

auto main(int argc, char** argv) -> int {
  using ut::operator""_test;

  tfc::base::init(argc, argv);

  "example config mock"_test = [] {
    asio::io_context ctx{};
    auto dbus{ std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system()) };
    testing::NiceMock<tfc::confman::mock_config<std::string>> config{ dbus, "some_key" };
    std::string presumed_value{ "my_value" };
    EXPECT_CALL(config, value).Times(1).WillOnce(testing::ReturnRef(presumed_value));
    auto const& expected_value{ config.value() };
    ut::expect(presumed_value == expected_value);
  };

  "example file storage mock"_test = [] {
    asio::io_context ctx{};
    testing::NiceMock<tfc::confman::mock_file_storage<std::string>> config{ ctx, "some_file_path" };
    std::string presumed_value{ "my_value" };
    EXPECT_CALL(config, value).Times(1).WillOnce(testing::ReturnRef(presumed_value));
    auto const& expected_value{ config.value() };
    ut::expect(presumed_value == expected_value);
  };

  // Remember to inject mocks to the actual use case with templates

  return EXIT_SUCCESS;
}
