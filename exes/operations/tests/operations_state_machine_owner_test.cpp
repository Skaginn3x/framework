#include <gmock/gmock.h>
#include <boost/asio/io_context.hpp>
#include <boost/ut.hpp>

#include <tfc/mocks/ipc.hpp>
#include <tfc/progbase.hpp>

#include "details/state_machine_owner_impl.hpp"

namespace asio = boost::asio;
namespace ut = boost::ut;
using ut::operator""_test;
using ut::operator|;
namespace events = tfc::operation::detail::events;

template <typename type, typename... type_policies>
struct state_machine_mock {
  template <typename... args>
    requires requires { requires(!std::is_same_v<state_machine_mock, args> && ...); }
  explicit state_machine_mock(args&&...) {}

  MOCK_METHOD(bool, process_set_starting, (), (const));        // NOLINT
  MOCK_METHOD(bool, process_run_button, (), (const));          // NOLINT
  MOCK_METHOD(bool, process_starting_timeout, (), (const));    // NOLINT
  MOCK_METHOD(bool, process_starting_finished, (), (const));   // NOLINT
  MOCK_METHOD(bool, process_set_stopped, (), (const));         // NOLINT
  MOCK_METHOD(bool, process_stopping_timeout, (), (const));    // NOLINT
  MOCK_METHOD(bool, process_stopping_finished, (), (const));   // NOLINT
  MOCK_METHOD(bool, process_cleaning_button, (), (const));     // NOLINT
  MOCK_METHOD(bool, process_set_cleaning, (), (const));        // NOLINT
  MOCK_METHOD(bool, process_set_emergency, (), (const));       // NOLINT
  MOCK_METHOD(bool, process_emergency_on, (), (const));        // NOLINT
  MOCK_METHOD(bool, process_emergency_off, (), (const));       // NOLINT
  MOCK_METHOD(bool, process_fault_on, (), (const));            // NOLINT
  MOCK_METHOD(bool, process_set_fault, (), (const));           // NOLINT
  MOCK_METHOD(bool, process_fault_off, (), (const));           // NOLINT
  MOCK_METHOD(bool, process_maintenance_button, (), (const));  // NOLINT
  MOCK_METHOD(bool, process_set_maintenance, (), (const));     // NOLINT

  template <typename event_t>
  auto process_event(event_t const&) -> bool {
    // clang-format off
    if constexpr (std::is_same_v<event_t, events::set_starting>) { return process_set_starting(); }
    else if constexpr (std::is_same_v<event_t, events::run_button>) { return process_run_button(); }
    else if constexpr (std::is_same_v<event_t, events::starting_timeout>) { return process_starting_timeout(); }
    else if constexpr (std::is_same_v<event_t, events::starting_finished>) { return process_starting_finished(); }
    else if constexpr (std::is_same_v<event_t, events::set_stopped>) { return process_set_stopped(); }
    else if constexpr (std::is_same_v<event_t, events::stopping_timeout>) { return process_stopping_timeout(); }
    else if constexpr (std::is_same_v<event_t, events::stopping_finished>) { return process_stopping_finished(); }
    else if constexpr (std::is_same_v<event_t, events::cleaning_button>) { return process_cleaning_button(); }
    else if constexpr (std::is_same_v<event_t, events::set_cleaning>) { return process_set_cleaning(); }
    else if constexpr (std::is_same_v<event_t, events::set_emergency>) { return process_set_emergency(); }
    else if constexpr (std::is_same_v<event_t, events::emergency_on>) { return process_emergency_on(); }
    else if constexpr (std::is_same_v<event_t, events::emergency_off>) { return process_emergency_off(); }
    else if constexpr (std::is_same_v<event_t, events::fault_on>) { return process_fault_on(); }
    else if constexpr (std::is_same_v<event_t, events::set_fault>) { return process_set_fault(); }
    else if constexpr (std::is_same_v<event_t, events::fault_off>) { return process_fault_off(); }
    else if constexpr (std::is_same_v<event_t, events::maintenance_button>) { return process_maintenance_button(); }
    else if constexpr (std::is_same_v<event_t, events::set_maintenance>) { return process_set_maintenance(); }
    else {
      []<bool flag = false>() {
        static_assert(flag, "Unsupported event type");
      }();
    }
    // clang-format on
  }
};

struct test_instance {
  asio::io_context ctx{};
  std::shared_ptr<sdbusplus::asio::connection> dbus{ std::make_shared<sdbusplus::asio::connection>(ctx) };
  tfc::operation::state_machine_owner<tfc::ipc::mock_signal, tfc::ipc::mock_slot, state_machine_mock> owner{ ctx, dbus };
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  ::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleMock();

  using enum tfc::operation::mode_e;

  "set_stopped"_test = [](auto mode) {
    test_instance instance{};
    EXPECT_CALL(*instance.owner.sm(), process_set_stopped()).Times(1);
    [[maybe_unused]] auto foo{ instance.owner.set_mode(mode) };
  } | std::vector{ stopping, stopped };

  "set_starting"_test = [](auto mode) {
    test_instance instance{};
    EXPECT_CALL(*instance.owner.sm(), process_set_starting()).Times(1);
    [[maybe_unused]] auto foo{ instance.owner.set_mode(mode) };
  } | std::vector{ starting, running };

  "set_cleaning"_test = [] {
    test_instance instance{};
    EXPECT_CALL(*instance.owner.sm(), process_set_cleaning()).Times(1);
    [[maybe_unused]] auto foo{ instance.owner.set_mode(cleaning) };
  };

  "set_emergency"_test = [] {
    test_instance instance{};
    EXPECT_CALL(*instance.owner.sm(), process_set_emergency()).Times(1);
    [[maybe_unused]] auto foo{ instance.owner.set_mode(emergency) };
  };

  "set_fault"_test = [] {
    test_instance instance{};
    EXPECT_CALL(*instance.owner.sm(), process_set_fault()).Times(1);
    [[maybe_unused]] auto foo{ instance.owner.set_mode(fault) };
  };

  "set_maintenance"_test = [] {
    test_instance instance{};
    EXPECT_CALL(*instance.owner.sm(), process_set_maintenance()).Times(1);
    [[maybe_unused]] auto foo{ instance.owner.set_mode(maintenance) };
  };

  "enter_stopped emits stopped"_test = [] {
    test_instance instance{};
    EXPECT_CALL(instance.owner.stopped_signal(), async_send_cb(true, testing::_)).Times(1);
    instance.owner.enter_stopped();
  };
  "leave_stopped emits stopped"_test = [] {
    test_instance instance{};
    EXPECT_CALL(instance.owner.stopped_signal(), async_send_cb(false, testing::_)).Times(1);
    instance.owner.leave_stopped();
  };

  "enter_starting emits starting"_test = [] {
    test_instance instance{};
    EXPECT_CALL(instance.owner.starting_signal(), async_send_cb(true, testing::_)).Times(1);
    instance.owner.enter_starting();
  };
  "leave_starting emits starting"_test = [] {
    test_instance instance{};
    EXPECT_CALL(instance.owner.starting_signal(), async_send_cb(false, testing::_)).Times(1);
    instance.owner.leave_starting();
  };
  "starting timer emits starting_timeout"_test = [] {
    test_instance instance{};
    EXPECT_CALL(*instance.owner.sm(), process_starting_timeout()).Times(1);
    instance.owner.on_starting_timer_expired({});

    EXPECT_CALL(*instance.owner.sm(), process_starting_timeout()).Times(0);
    instance.owner.on_starting_timer_expired(std::make_error_code(std::errc::operation_canceled));
  };

  "enter_running emits running"_test = [] {
    test_instance instance{};
    EXPECT_CALL(instance.owner.running_signal(), async_send_cb(true, testing::_)).Times(1);
    instance.owner.enter_running();
  };
  "leave_running emits running"_test = [] {
    test_instance instance{};
    EXPECT_CALL(instance.owner.running_signal(), async_send_cb(false, testing::_)).Times(1);
    instance.owner.leave_running();
  };

  "enter_stoppping emits stopping"_test = [] {
    test_instance instance{};
    EXPECT_CALL(instance.owner.stopping_signal(), async_send_cb(true, testing::_)).Times(1);
    instance.owner.enter_stopping();
  };
  "leave_stopping emits stopping"_test = [] {
    test_instance instance{};
    EXPECT_CALL(instance.owner.stopping_signal(), async_send_cb(false, testing::_)).Times(1);
    instance.owner.leave_stopping();
  };
  "stopping timer emits stopping_timeout"_test = [] {
    test_instance instance{};
    EXPECT_CALL(*instance.owner.sm(), process_stopping_timeout()).Times(1);
    instance.owner.on_stopping_timer_expired({});

    EXPECT_CALL(*instance.owner.sm(), process_stopping_timeout()).Times(0);
    instance.owner.on_stopping_timer_expired(std::make_error_code(std::errc::operation_canceled));
  };

  "enter_cleaning emits cleaning"_test = [] {
    test_instance instance{};
    EXPECT_CALL(instance.owner.cleaning_signal(), async_send_cb(true, testing::_)).Times(1);
    instance.owner.enter_cleaning();
  };
  "leave_cleaning emits cleaning"_test = [] {
    test_instance instance{};
    EXPECT_CALL(instance.owner.cleaning_signal(), async_send_cb(false, testing::_)).Times(1);
    instance.owner.leave_cleaning();
  };

  "transition call owner"_test = [] {
    test_instance instance{};
    bool called{};
    instance.owner.on_new_mode([&called](auto new_mode, auto old_mode) {
      called = true;
      ut::expect(new_mode == starting);
      ut::expect(old_mode == running);
    });
    instance.owner.transition(starting, running);
    ut::expect(called);
  };
  "transition set mode signals"_test = [] {
    test_instance instance{};
    EXPECT_CALL(instance.owner.mode_signal(), async_send_cb(std::to_underlying(starting), testing::_)).Times(1);
    std::string const starting_str{ enum_name(starting) };
    EXPECT_CALL(instance.owner.mode_str_signal(), async_send_cb(starting_str, testing::_)).Times(1);
    instance.owner.transition(starting, running);
  };

  "starting_finished_new_state rising edge emits starting_finished"_test = [] {
    test_instance instance{};
    EXPECT_CALL(*instance.owner.sm(), process_starting_finished()).Times(1);
    instance.owner.starting_finished_new_state(true);

    EXPECT_CALL(*instance.owner.sm(), process_starting_finished()).Times(0);
    instance.owner.starting_finished_new_state(false);
  };

  "stopping_finished_new_state rising edge emits stopping_finished"_test = [] {
    test_instance instance{};
    EXPECT_CALL(*instance.owner.sm(), process_stopping_finished()).Times(1);
    instance.owner.stopping_finished_new_state(true);

    EXPECT_CALL(*instance.owner.sm(), process_stopping_finished()).Times(0);
    instance.owner.stopping_finished_new_state(false);
  };

  "running_new_state rising edge emits running"_test = [] {
    test_instance instance{};
    EXPECT_CALL(*instance.owner.sm(), process_run_button()).Times(1);
    instance.owner.running_new_state(true);

    EXPECT_CALL(*instance.owner.sm(), process_run_button()).Times(0);
    instance.owner.running_new_state(false);
  };

  "cleaning_new_state rising edge emits cleaning"_test = [] {
    test_instance instance{};
    EXPECT_CALL(*instance.owner.sm(), process_cleaning_button()).Times(1);
    instance.owner.cleaning_new_state(true);

    EXPECT_CALL(*instance.owner.sm(), process_cleaning_button()).Times(0);
    instance.owner.cleaning_new_state(false);
  };

  "maintenance_new_state rising edge emits maintenance"_test = [] {
    test_instance instance{};
    EXPECT_CALL(*instance.owner.sm(), process_maintenance_button()).Times(1);
    instance.owner.maintenance_new_state(true);

    EXPECT_CALL(*instance.owner.sm(), process_maintenance_button()).Times(0);
    instance.owner.maintenance_new_state(false);
  };

  return EXIT_SUCCESS;
}
