#include <boost/ut.hpp>
#include <boost/asio/io_context.hpp>
#include <gmock/gmock.h>

#include <tfc/progbase.hpp>
#include <tfc/mocks/ipc.hpp>

#include "sensor_control_impl.hpp"

namespace asio = boost::asio;
namespace ut = boost::ut;
using ut::operator""_test;
using ut::operator>>;
using ut::fatal;
namespace item = tfc::ipc::item;
namespace events = tfc::sensor::control::events;

template <typename type, typename... type_policies>
struct state_machine_mock {
  explicit state_machine_mock(auto&& ...) {}

  MOCK_METHOD(void, process_event_sensor_active, (), (const));
  MOCK_METHOD(void, process_event_info, (), (const));
  MOCK_METHOD(void, process_discharge, (), (const));
  MOCK_METHOD(void, process_complete, (), (const));

  template <typename event_t>
  void process_event(event_t const&) {
    if constexpr (std::is_same_v<event_t, events::sensor_active>) {
      process_event_sensor_active();
    }
    else if constexpr (std::is_same_v<event_t, events::new_info>) {
      process_event_info();
    }
    else if constexpr (std::is_same_v<event_t, events::discharge>) {
      process_discharge();
    }
    else if constexpr (std::is_same_v<event_t, events::complete>) {
      process_complete();
    }
    else {
      []<bool flag = false>() {
        static_assert(flag, "Unsupported event type");
      }();
    }
  }
};

struct test_instance {
  asio::io_context ctx{};
  tfc::sensor_control<tfc::ipc::mock_signal, tfc::ipc::mock_slot, state_machine_mock> ctrl{ ctx };
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  ::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleMock();

  "on_idle stop motor"_test = []{
    test_instance instance{};
    EXPECT_CALL(instance.ctrl.motor_signal(), async_send_cb(0.0, testing::_)).Times(1);
    instance.ctrl.enter_idle();
  };

  "on_awaiting sensor start motor"_test = [] {
    test_instance instance{};
    EXPECT_CALL(instance.ctrl.motor_signal(), async_send_cb(100.0, testing::_)).Times(1);
    instance.ctrl.enter_awaiting_sensor();
  };

  "leave_awaiting sensor stop motor"_test = [] {
    test_instance instance{};
    EXPECT_CALL(instance.ctrl.motor_signal(), async_send_cb(0.0, testing::_)).Times(1);
    instance.ctrl.leave_awaiting_sensor();
  };

  "discharging start motor"_test = [] {
    test_instance instance{};
    EXPECT_CALL(instance.ctrl.motor_signal(), async_send_cb(100.0, testing::_)).Times(1);
    instance.ctrl.enter_discharging();
  };

  "on_sensor true emit sensor_active"_test = [] {
    test_instance instance{};
    EXPECT_CALL(*instance.ctrl.state_machine(), process_event_sensor_active()).Times(1);
    instance.ctrl.on_sensor(true);
  };

  "on_sensor false emit complete"_test = [] {
    test_instance instance{};
    EXPECT_CALL(*instance.ctrl.state_machine(), process_complete()).Times(1);
    instance.ctrl.on_sensor(false);
  };

  "forget invalid json requests"_test = [] {
    test_instance instance{};
    EXPECT_CALL(*instance.ctrl.state_machine(), process_event_info()).Times(0);
    instance.ctrl.on_discharge_request("\"invalid json");
    ut::expect(!instance.ctrl.queued_item().has_value());
  };

  "forget invalid item requests"_test = [] {
    test_instance instance{};
    EXPECT_CALL(*instance.ctrl.state_machine(), process_event_info()).Times(0);
    instance.ctrl.on_discharge_request(R"({"invalid": "json"})");
    ut::expect(!instance.ctrl.queued_item().has_value());
  };

  "store valid item requests"_test = [] {
    test_instance instance{};
    EXPECT_CALL(*instance.ctrl.state_machine(), process_event_info()).Times(1);
    auto new_item{ item::make() };
    instance.ctrl.on_discharge_request(new_item.to_json());
    auto presumed_item{ instance.ctrl.queued_item() };
    ut::expect(presumed_item.has_value() >> fatal);
    ut::expect(presumed_item->item_id == new_item.item_id);
    ut::expect(presumed_item->entry_timestamp == new_item.entry_timestamp);
    ut::expect(presumed_item.value() == new_item);
  };

  "enter_awaiting_discharge asks for discharge downstream with stored item"_test = [] {
    test_instance instance{};
    auto new_item{ item::make() };
    auto expected_json{ new_item.to_json() };
    instance.ctrl.set_queued_item(std::move(new_item));
    EXPECT_CALL(instance.ctrl.discharge_signal(), async_send_cb(expected_json, testing::_)).Times(1);
    instance.ctrl.enter_awaiting_discharge();
    // queued item should be cleared
    ut::expect(!instance.ctrl.queued_item().has_value());
  };

  "enter_awaiting_discharge asks for discharge downstream with new item if stored is non-existent"_test = [] {
    test_instance instance{};
    EXPECT_CALL(instance.ctrl.discharge_signal(), async_send_cb(testing::_, testing::_)).Times(1);
    instance.ctrl.enter_awaiting_discharge();
  };

  "awaiting_sensor allows discharge"_test = [] {
    test_instance instance{};
    EXPECT_CALL(instance.ctrl.discharge_allowance_signal(), async_send_cb(true, testing::_)).Times(1);
    instance.ctrl.enter_awaiting_sensor();
  };

  "leave_awaiting_sensor revokes discharge allowance"_test = [] {
    test_instance instance{};
    EXPECT_CALL(instance.ctrl.discharge_allowance_signal(), async_send_cb(false, testing::_)).Times(1);
    instance.ctrl.leave_awaiting_sensor();
  };

  "may_discharge emits event discharge"_test = [] {
    test_instance instance{};
    EXPECT_CALL(*instance.ctrl.state_machine(), process_discharge()).Times(1);
    instance.ctrl.on_may_discharge(true);
  };

  "enter_awaiting_discharge emits event discharge if it may discharge"_test = [] {
    test_instance instance{};
    std::optional<bool> const value{ true };
    ON_CALL(instance.ctrl.may_discharge_slot(), value()).WillByDefault(testing::ReturnRef(value));
    EXPECT_CALL(*instance.ctrl.state_machine(), process_discharge()).Times(1);
    instance.ctrl.enter_awaiting_discharge();
  };


  return EXIT_SUCCESS;
}



