#include <boost/asio.hpp>
#include <boost/ut.hpp>
#include <boost/sml.hpp>
#include <gmock/gmock.h>

#include <tfc/progbase.hpp>
#include <tfc/mocks/ipc.hpp>

#include <sensor_control.hpp>
#include <sensor_control_impl.hpp>

namespace asio = boost::asio;
namespace control = tfc::sensor::control;
namespace ut = boost::ut;
namespace sml = boost::sml;
using ut::operator""_test;
using sml::operator""_s;
using sml::state;

struct two_connectives {
  asio::io_context ctx{};
  using sensor_control_t = tfc::sensor_control<tfc::ipc::mock_signal, tfc::ipc::mock_slot>;
  sensor_control_t first{ ctx, "first" };
  sensor_control_t second{ ctx, "second" };
  using inner_sm_t = control::state_machine<sensor_control_t>;
  auto init() {
    first.on_running();
    second.on_running();
    ON_CALL(second.discharge_allowance_signal(), async_send_cb).WillByDefault([this](bool const& value, std::function<void(std::error_code, std::size_t)>) {
      first.may_discharge_slot().callback(value);
    });
    ON_CALL(first.discharge_signal(), async_send_cb).WillByDefault([this](std::string const& value, std::function<void(std::error_code, std::size_t)>) {
      second.discharge_request_slot().callback(value);
    });
    ut::expect(first.state_machine()->is<decltype(state<inner_sm_t>)>(state<control::states::idle>));
    ut::expect(second.state_machine()->is<decltype(state<inner_sm_t>)>(state<control::states::idle>));
  }
};

using inner_sm_state_t = decltype(state<two_connectives::inner_sm_t>);

int main (int argc, char** argv) {
  auto substitute_argv = std::array{ "sensor_control_integration_test", "--log-level=trace", "--stdout" };

  tfc::base::init(substitute_argv.size(), substitute_argv.data());

  "happy path"_test = [] {
    two_connectives test{};
    auto item{ tfc::ipc::item::make() };
    test.init();

    // Send discharge request
    test.first.discharge_request_slot().callback(item.to_json());
    ut::expect(test.first.state_machine()->is<inner_sm_state_t>(state<control::states::awaiting_sensor>));
    ut::expect(test.second.state_machine()->is<inner_sm_state_t>(state<control::states::idle>));

    // Item appears in sensor and asks downstream to accept the item,
    // since downstream is empty it will immediately accept
    test.first.sensor_slot().callback(true);
    ut::expect(test.first.state_machine()->is<inner_sm_state_t>(state<control::states::discharging>));
    ut::expect(test.second.state_machine()->is<inner_sm_state_t>(state<control::states::awaiting_sensor>));

  };

  return 0;
}

