#include <boost/asio.hpp>
#include <boost/ut.hpp>

#include <tfc/ec/devices/beckhoff/EL1xxx_impl.hpp>
#include <tfc/ec/devices/beckhoff/EL2xxx_impl.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc/details/dbus_client_iface_mock.hpp>
#include <tfc/progbase.hpp>
#include <tfc/mocks/ipc.hpp>

namespace asio = boost::asio;
namespace ut = boost::ut;
namespace beckhoff = tfc::ec::devices::beckhoff;

using ut::operator""_test;
using ut::operator|;
using tfc::ipc::slot;
using tfc::ipc::details::type_bool;
using tfc::ipc_ruler::ipc_manager_client_mock;
using ut::expect;

template <typename device_t>
struct test_vars {
  asio::io_context ctx{};
  ipc_manager_client_mock connect_interface{};
  device_t device;
};
auto main(int argc, const char* argv[]) -> int {
  ut::detail::cfg::parse(argc, argv);
  tfc::base::init(argc, argv);  // todo should we make relaxed_init

  ::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleMock();

  [[maybe_unused]] ut::suite<"EL1xxx"> el1xxx_suite = [] {  // NOLINT
    "2 output"_test = [] {
      test_vars<beckhoff::el1002<ipc_manager_client_mock, tfc::ipc::mock_signal>> vars{ .device = { vars.ctx, vars.connect_interface, 42 } };
      std::array<std::byte, 1> buffer{ std::byte{ 0b11 } };
      auto const& transmitters{ vars.device.transmitters() };
      EXPECT_CALL(*transmitters.at(0), async_send_cb(true, testing::_)).Times(1);
      EXPECT_CALL(*transmitters.at(1), async_send_cb(true, testing::_)).Times(1);
      vars.device.process_data(buffer, {});
      buffer = { std::byte{ 0b00 } };
      EXPECT_CALL(*transmitters.at(0), async_send_cb(false, testing::_)).Times(1);
      EXPECT_CALL(*transmitters.at(1), async_send_cb(false, testing::_)).Times(1);
      vars.device.process_data(buffer, {});

      // Only calls when value changes
      buffer = { std::byte{ 0b01 } };
      EXPECT_CALL(*transmitters.at(0), async_send_cb(true, testing::_)).Times(1);
      EXPECT_CALL(*transmitters.at(1), async_send_cb(false, testing::_)).Times(0);
      vars.device.process_data(buffer, {});
    };
  };

  [[maybe_unused]] ut::suite<"EL2xxx"> el2xxx_suite = [] {  // NOLINT
    static constexpr auto process_data{ []<std::size_t size>(auto& el2xxx, std::bitset<size> test_value) {
      for (std::size_t idx{ 0 }; idx < test_value.size(); idx++) {
        el2xxx.set_output(idx, test_value[idx]);
      }
      static constexpr auto array_size{ std::max(1UZ, size / 8) };  // 1 if size == 4 || size == 8, 2 if size == 16
      std::array<std::byte, array_size> buffer{};
      el2xxx.process_data({}, buffer);
      std::uint16_t buffer_out{ std::to_integer<uint8_t>(buffer[0]) };  // NOLINT
      if constexpr (array_size == 2) {
        buffer_out <<= 8;
        buffer_out += std::to_integer<uint8_t>(buffer[1]);
      }
      decltype(test_value) buffer_bitset{ buffer_out };
      return buffer_bitset;
    } };

    "4 inputs"_test = [](std::bitset<4> test_value) {
      test_vars<beckhoff::el2004<ipc_manager_client_mock>> vars{ .device = { vars.ctx, vars.connect_interface, 42 } };
      auto bitset{ process_data(vars.device, test_value) };
      ut::expect(bitset == test_value);
    } | std::vector<std::bitset<4>>{ 0b1011, 0b1010, 0b1111, 0b0000 };
    "8 inputs"_test = [](std::bitset<8> test_value) {
      test_vars<beckhoff::el2008<ipc_manager_client_mock>> vars{ .device = { vars.ctx, vars.connect_interface, 42 } };
      auto bitset{ process_data(vars.device, test_value) };
      ut::expect(bitset == test_value);
    } | std::vector<std::bitset<8>>{ 0b10101010, 0b11110000, 0b11111111, 0b00000000 };
    "16 inputs"_test = [](std::bitset<16> test_value) {
      test_vars<beckhoff::el2809<ipc_manager_client_mock>> vars{ .device = { vars.ctx, vars.connect_interface, 42 } };
      auto bitset{ process_data(vars.device, test_value) };
      ut::expect(bitset == test_value);
    } | std::vector<std::bitset<16>>{ 0b1010101010101010, 0b1111000011110000, 0b1111111111111111, 0b0000000000000000 };
  };

  return static_cast<int>(ut::cfg<>.run({ .report_errors = true }));
}
