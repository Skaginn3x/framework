#include <boost/asio.hpp>
#include <boost/ut.hpp>
#include <gmock/gmock.h>

#include <tfc/progbase.hpp>
#include <tfc/mocks/ipc.hpp>

#include <sensor_control.hpp>
#include <sensor_control_impl.hpp>

namespace asio = boost::asio;
namespace ut = boost::ut;
using ut::operator""_test;

struct two_connectives {
  asio::io_context ctx{};
  tfc::sensor_control<tfc::ipc::mock_signal, tfc::ipc::mock_slot> first{ ctx };
  tfc::sensor_control<tfc::ipc::mock_signal, tfc::ipc::mock_slot> second{ ctx };
  auto connect_first_to_second() {
    ON_CALL(first.discharge_allowance_signal(), async_send_cb).WillByDefault([this](bool const& value, std::function<void(std::error_code, std::size_t)>) {
      second.may_discharge_slot().callback(value);
    });
    ON_CALL(first.discharge_signal(), async_send_cb).WillByDefault([this](std::string const& value, std::function<void(std::error_code, std::size_t)>) {
      second.discharge_request_slot().callback(value);
    });

  }
};

int main (int argc, char** argv) {
  auto substitute_argv = std::array{ "sensor_control_integration_test", "--log-level=trace", "--stdout" };

  tfc::base::init(substitute_argv.size(), substitute_argv.data());

  "two connectives"_test = [] {
    two_connectives test{};
    test.connect_first_to_second();
    test.first.sensor_slot().callback(true);

  };

  return 0;
}

