#include <boost/asio.hpp>
#include <boost/ut.hpp>

#include <tfc/confman/file_storage.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc/details/dbus_client_iface.hpp>
#include <tfc/ipc/details/dbus_client_iface_mock.hpp>
#include <tfc/ipc/details/dbus_server_iface.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;
namespace ut = boost::ut;
using ut::operator""_test;

template <typename storage_t>
struct file_storage_mock {
  using type = storage_t;

  auto set_changed() const noexcept -> std::error_code {
    set_changed_cb();
    return {};
  }

  auto operator->() const noexcept -> storage_t const* { return std::addressof(value()); }

  auto value() const noexcept -> storage_t const& { return storage_; }

  auto access() noexcept -> storage_t& { return storage_; }

  using change = tfc::confman::detail::change<file_storage_mock>;

  auto make_change() { return change{ *this }; }

  storage_t storage_{};
  std::function<void(void)> set_changed_cb = []() {};
};

using signal_storage_t = file_storage_mock<std::unordered_map<std::string, tfc::ipc_ruler::signal>>;
using slot_storage_t = file_storage_mock<std::unordered_map<std::string, tfc::ipc_ruler::slot>>;
using manager_t = tfc::ipc_ruler::ipc_manager<signal_storage_t, slot_storage_t>;

struct test_instance {
  asio::io_context ctx{};
  signal_storage_t signals{};
  slot_storage_t slots{};
  tfc::ipc_ruler::ipc_manager_server<signal_storage_t, slot_storage_t> ipc_manager_server{
    ctx, std::make_unique<manager_t>(signals, slots)
  };
  tfc::ipc_ruler::ipc_manager_client ipc_manager_client{ ctx };
  bool ran{};

  int test_value = 0;
  auto increment([[maybe_unused]] sdbusplus::message_t& msg) -> void { test_value++; }
};

class test_class {
  int test_value = 0;

public:
  auto increment([[maybe_unused]] sdbusplus::message_t& msg) -> void { test_value++; }

  auto value() -> int { return test_value; }
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  "ipc_manager correctness check"_test = []() {
    signal_storage_t signals{};
    slot_storage_t slots{};
    auto ipc_manager = std::make_unique<manager_t>(signals, slots);
    ipc_manager->register_signal("some_signal", "Test signal description", tfc::ipc::details::type_e::_bool);
    ut::expect(ipc_manager->get_all_signals().size() == 1);
    ut::expect(ipc_manager->get_all_slots().empty());
    ut::expect(ipc_manager->get_all_signals()[0].description == "Test signal description");
    ut::expect(ipc_manager->get_all_signals()[0].name == "some_signal");

    signals.storage_ = {};
    ut::expect(ipc_manager->get_all_signals().empty());
  };

  "get signals empty"_test = [] {
    test_instance instance{};
    // Check if the correct empty list is reported for signals
    instance.ipc_manager_client.signals([&instance](auto signals) {
      ut::expect(signals.empty());
      instance.ran = true;
    });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    ut::expect(instance.ran);
  };

  "get slots empty"_test = [] {
    test_instance instance{};
    // Check if the correct empty list is reported for slots
    instance.ipc_manager_client.slots([&instance](auto slots) {
      ut::expect(slots.empty());
      instance.ran = true;
    });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    ut::expect(instance.ran);
  };

  "register signal"_test = [] {
    test_instance instance{};
    instance.ipc_manager_client.register_signal("test_signal", "", tfc::ipc::details::type_e::_string,
                                                [&instance](const std::error_code& err) {
                                                  ut::expect(!err) << err.message();
                                                  instance.ran = true;
                                                });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    ut::expect(instance.ran);

    // check that the signal appears
    instance.ran = false;
    instance.ipc_manager_client.signals([&instance](auto signals) {
      ut::expect(!signals.empty());
      ut::expect(signals.size() == 1);
      if (signals.size() == 1) {
        ut::expect(signals[0].name == "test_signal");
      }
      instance.ran = true;
    });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    ut::expect(instance.ran);
  };

  "register slot"_test = [] {
    test_instance instance{};
    instance.ipc_manager_client.register_slot("test_slot", "", tfc::ipc::details::type_e::_string,
                                              [&instance](const std::error_code& err) {
                                                ut::expect(!err) << err.message();
                                                instance.ran = true;
                                              });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    ut::expect(instance.ran);

    // Check that the slot appears
    instance.ran = false;
    instance.ipc_manager_client.slots([&instance](auto slots) {
      ut::expect(!slots.empty());
      ut::expect(slots.size() == 1);
      if (slots.size() == 1) {
        ut::expect(slots[0].name == "test_slot");
      }
      instance.ran = true;
    });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    ut::expect(instance.ran);
  };

  "connection change subscription"_test = []() {
    test_instance instance{};

    instance.ipc_manager_client.register_signal("test_signal", "", tfc::ipc::details::type_e::_string, [](const auto&) {});
    instance.ctx.run_for(std::chrono::milliseconds(5));
    instance.ipc_manager_client.register_slot("test_slot", "", tfc::ipc::details::type_e::_string, [](const auto&) {});
    instance.ctx.run_for(std::chrono::milliseconds(5));
    // Register a method for connection change callback
    instance.ipc_manager_client.register_connection_change_callback("test_slot", [&instance](std::string_view signal_name) {
      ut::expect(signal_name == "test_signal");
      instance.ran = true;
    });
    instance.ipc_manager_client.connect("test_slot", "test_signal", [](const std::error_code& err) { ut::expect(!err); });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    ut::expect(instance.ran);
  };
  "Check that re-registering a communication channel only changes last_registered "_test = []() {
    test_instance instance{};

    // Check if the correct empty list is reported for signals

    bool registered_signal = false;
    bool registered_slot = false;
    // Register a signal and slot for the first time
    instance.ipc_manager_client.register_signal("signal_register_retest", "", tfc::ipc::details::type_e::_string,
                                                [&](const std::error_code& err) {
                                                  ut::expect(!err);
                                                  registered_signal = true;
                                                });

    // Expect to get a callback
    bool got_callback = false;
    instance.ipc_manager_client.register_connection_change_callback("slot_register_retest",
                                                                    [&](const std::string_view&) { got_callback = true; });
    instance.ipc_manager_client.register_slot("slot_register_retest", "", tfc::ipc::details::type_e::_string,
                                              [&](const std::error_code& err) {
                                                ut::expect(!err);
                                                registered_slot = true;
                                              });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    ut::expect(got_callback);
    ut::expect(registered_signal);
    ut::expect(registered_slot);

    // Connect the slot to the signal
    instance.ipc_manager_client.connect("slot_register_retest", "signal_register_retest",
                                        [](const std::error_code& err) { ut::expect(!err); });

    instance.ctx.run_for(std::chrono::milliseconds(5));

    tfc::ipc_ruler::signal signal_copy;
    tfc::ipc_ruler::slot slot_copy;
    // Copy the registered slot and signal and then re-register and verify the information is intact
    instance.ipc_manager_client.signals([&](auto signals) {
      for (auto& signal : signals) {
        if (signal.name == "signal_register_retest") {
          signal_copy = signal;
          return;
        }
      }
      ut::expect(false);
    });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    instance.ipc_manager_client.slots([&](auto slots) {
      for (auto& slot : slots) {
        if (slot.name == "slot_register_retest") {
          slot_copy = slot;
          return;
        }
      }
      ut::expect(false);
    });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    ut::expect(!signal_copy.name.empty());
    ut::expect(!slot_copy.name.empty());

    registered_signal = false;
    registered_slot = false;
    // Register a signal and slot for the second time
    instance.ipc_manager_client.register_signal("signal_register_retest", "", tfc::ipc::details::type_e::_string,
                                                [&](const std::error_code& err) {
                                                  ut::expect(!err);
                                                  registered_signal = true;
                                                });
    instance.ipc_manager_client.register_slot("slot_register_retest", "", tfc::ipc::details::type_e::_string,
                                              [&](const std::error_code& err) {
                                                ut::expect(!err);
                                                registered_slot = true;
                                              });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    ut::expect(registered_signal);
    ut::expect(registered_slot);

    tfc::ipc_ruler::signal signal_copy2;
    tfc::ipc_ruler::slot slot_copy2;
    // Copy the registered slot and signal and then re-register and verify the information is intact
    instance.ipc_manager_client.signals([&](auto signals) {
      for (auto& signal : signals) {
        if (signal.name == "signal_register_retest") {
          signal_copy2 = signal;
          return;
        }
      }
      ut::expect(false);
    });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    instance.ipc_manager_client.slots([&](auto slots) {
      for (auto& slot : slots) {
        if (slot.name == "slot_register_retest") {
          slot_copy2 = slot;
          return;
        }
      }
      ut::expect(false);
    });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    ut::expect(!signal_copy2.name.empty());
    ut::expect(!slot_copy2.name.empty());

    // Compare the two copies and verify that only the last_registered value changed
    ut::expect(signal_copy.name == signal_copy2.name);
    ut::expect(signal_copy.created_at == signal_copy2.created_at);
    ut::expect(signal_copy.created_by == signal_copy2.created_by);
    ut::expect(signal_copy.type == signal_copy2.type);
    ut::expect(signal_copy.last_registered != signal_copy2.last_registered);

    ut::expect(slot_copy.name == slot_copy2.name);
    ut::expect(slot_copy.created_at == slot_copy2.created_at);
    ut::expect(slot_copy.created_by == slot_copy2.created_by);
    ut::expect(slot_copy.type == slot_copy2.type);
    ut::expect(slot_copy.modified_by == slot_copy2.modified_by);
    ut::expect(slot_copy.connected_to == slot_copy2.connected_to);
    ut::expect(slot_copy.last_registered != slot_copy2.last_registered);
  };

  "Test ipc communication connection and disconnection with mocking bool"_test = []() {
    asio::io_context isolated_ctx{};

    std::array<bool, 3> test_values{ true, false, true };
    uint8_t invocation{};
    bool ignore_first{ true };

    tfc::ipc_ruler::ipc_manager_client_mock mock_client;
    const tfc::ipc::slot<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&> slot(
        isolated_ctx, mock_client, "bool_slot", "", [&](bool value) {
          if (ignore_first) {
            ignore_first = false;
            return;
          }
          ut::expect(test_values.at(invocation++) == value);
          if (invocation == test_values.size()) {
            isolated_ctx.stop();
          }
        });
    tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&> sig(isolated_ctx, mock_client,
                                                                                                "bool_signal", "");

    mock_client.connect(mock_client.slots_[0].name, mock_client.signals_[0].name,
                        [&](const std::error_code& err) { ut::expect(!err); });

    asio::steady_timer timer{ isolated_ctx };
    timer.expires_after(std::chrono::milliseconds(10));
    timer.async_wait([&sig, &test_values](std::error_code) {
      for (auto const value : test_values) {
        sig.async_send(value, [](std::error_code, std::size_t) {});
      }
    });
    isolated_ctx.run_for(std::chrono::seconds(3));
    ut::expect(invocation == test_values.size()) << "got invoked: " << invocation;
  };
  "Test ipc communication connection and disconnection with mocking int"_test = []() {
    asio::io_context isolated_ctx{};

    tfc::ipc_ruler::ipc_manager_client_mock mock_client;

    uint8_t invocation{};
    bool ignore_first{ true };
    std::array<std::int64_t, 3> test_values{ 25, 1337, 42 };

    const tfc::ipc::slot<tfc::ipc::details::type_int, tfc::ipc_ruler::ipc_manager_client_mock&> slot(
        isolated_ctx, mock_client, "bool_slot", "", [&](int64_t value) {
          if (ignore_first) {
            ignore_first = false;
            return;
          }
          ut::expect(test_values.at(invocation++) == value);
          if (invocation == test_values.size()) {
            isolated_ctx.stop();
          }
        });
    tfc::ipc::signal<tfc::ipc::details::type_int, tfc::ipc_ruler::ipc_manager_client_mock&> sig(isolated_ctx, mock_client,
                                                                                               "bool_signal", "");

    mock_client.connect(mock_client.slots_[0].name, mock_client.signals_[0].name,
                        [](const std::error_code& err) { ut::expect(!err); });

    std::array<std::string, 1> slot_names = { "ipc_manager_test.def.int64_t.bool_slot" };

    mock_client.slots([&](auto slots) {
      std::size_t counter = 0;
      for (auto& i_slot : slots) {
        if (i_slot.name == slot_names[counter]) {
          ut::expect(i_slot.name == slot_names[counter]);
        } else {
          ut::expect(false);
        }
        counter++;
      }
    });

    std::array<std::string, 1> signal_names = { "ipc_manager_test.def.int64_t.bool_signal" };

    mock_client.signals([&](auto signals) {
      std::size_t counter = 0;
      for (auto& signal : signals) {
        if (signal.name == signal_names[counter]) {
          ut::expect(signal.name == signal_names[counter]);
        } else {
          ut::expect(false);
        }
        counter++;
      }
    });

    asio::steady_timer timer{ isolated_ctx };
    timer.expires_after(std::chrono::milliseconds(10));
    timer.async_wait([&sig, &test_values](std::error_code) {
      for (auto const value : test_values) {
        sig.async_send(value, [](std::error_code, std::size_t) {});
      }
    });
    isolated_ctx.run_for(std::chrono::seconds(3));
    ut::expect(invocation == test_values.size());
  };

  "Test callback functionality"_test = []() {
    test_class test_class_instance;

    asio::io_context isolated_ctx{};
    tfc::ipc_ruler::ipc_manager_client_mock mock_client;

    mock_client.register_properties_change_callback(std::bind_front(&test_class::increment, &test_class_instance));

    ut::expect(test_class_instance.value() == 0);

    tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&> signal(
        isolated_ctx, mock_client, "test_signal", "description2");

    isolated_ctx.run_for(std::chrono::milliseconds(20));

    ut::expect(test_class_instance.value() == 1);
  };

  "Testing callback functionality on IPC client"_test = []() {
    test_instance instance{};

    instance.ipc_manager_client.register_signal("test1", "", tfc::ipc::details::type_e::_string,
                                                [&](const std::error_code& err) { ut::expect(!err); });

    instance.ctx.run_for(std::chrono::milliseconds(20));

    std::unique_ptr<sdbusplus::bus::match::match> cb = instance.ipc_manager_client.register_properties_change_callback(
        std::bind_front(&test_instance::increment, &instance));

    instance.ctx.run_for(std::chrono::milliseconds(20));

    instance.ipc_manager_client.register_signal("test2", "", tfc::ipc::details::type_e::_string,
                                                [&](const std::error_code& err) { ut::expect(!err); });

    instance.ctx.run_for(std::chrono::milliseconds(20));

    ut::expect(instance.test_value == 1);
  };

  return EXIT_SUCCESS;
}
