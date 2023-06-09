#include <boost/asio.hpp>
#include <boost/ut.hpp>
#include <glaze/glaze.hpp>
#include <tfc/confman/file_storage.hpp>
#include <tfc/ipc.hpp>
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

  "ipc_manager check over dbus"_test = []() {
    test_instance instance{};
    // Check if the correct empty list is reported for signals
    bool ran = false;
    instance.ipc_manager_client.signals([&](auto v) {
      ut::expect(v.empty());
      ran = true;
    });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    ut::expect(ran);

    // Check if the correct empty list is reported for slots
    instance.ipc_manager_client.slots([&](auto v) {
      ut::expect(v.empty());
      ran = true;
    });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    ut::expect(ran);

    // add a signal
    ran = false;
    instance.ipc_manager_client.register_signal("test_signal", "", tfc::ipc::details::type_e::_string,
                                                [&](const std::error_code& err) {
                                                  ut::expect(!err) << err.message();
                                                  ran = true;
                                                });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    ut::expect(ran);

    // check that the signal appears
    ran = false;
    instance.ipc_manager_client.signals([&](auto v) {
      ut::expect(!v.empty());
      ut::expect(v.size() == 1);
      if (v.size() == 1) {
        ut::expect(v[0].name == "test_signal");
      }
      ran = true;
    });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    ut::expect(ran);

    // Register a slot
    ran = false;
    instance.ipc_manager_client.register_slot("test_slot", "", tfc::ipc::details::type_e::_string,
                                              [&](const std::error_code& err) {
                                                ut::expect(!err);
                                                ran = true;
                                              });

    // Check that the slot appears
    instance.ctx.run_for(std::chrono::milliseconds(5));
    ut::expect(ran);
    ran = false;
    instance.ipc_manager_client.slots([&](auto v) {
      ut::expect(!v.empty());
      ut::expect(v.size() == 1);
      if (v.size() == 1) {
        ut::expect(v[0].name == "test_slot");
      }
      ran = true;
    });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    ut::expect(ran);

    // Register a method for connection change callback
    ran = false;
    instance.ipc_manager_client.register_connection_change_callback("test_slot", [&](std::string_view signal_name) {
      ut::expect(signal_name == "test_signal");
      ran = true;
    });
    ut::expect(!ran);
    instance.ipc_manager_client.connect("test_slot", "test_signal", [](const std::error_code& err) { ut::expect(!err); });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    ut::expect(ran);
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
    instance.ipc_manager_client.signals([&](auto v) {
      for (auto& signal : v) {
        if (signal.name == "signal_register_retest") {
          signal_copy = signal;
          return;
        }
      }
      ut::expect(false);
    });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    instance.ipc_manager_client.slots([&](auto v) {
      for (auto& slot : v) {
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
    instance.ipc_manager_client.signals([&](auto v) {
      for (auto& signal : v) {
        if (signal.name == "signal_register_retest") {
          signal_copy2 = signal;
          return;
        }
      }
      ut::expect(false);
    });
    instance.ctx.run_for(std::chrono::milliseconds(5));
    instance.ipc_manager_client.slots([&](auto v) {
      for (auto& slot : v) {
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

    tfc::ipc_ruler::ipc_manager_client_mock mclient;
    tfc::ipc::slot<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock> slot(
        isolated_ctx, mclient, "bool_slot", "", [&](bool value) {
          if (ignore_first) {
            ignore_first = false;
            return;
          }
          ut::expect(test_values.at(invocation++) == value);
          if (invocation == test_values.size()) {
            isolated_ctx.stop();
          }
        });
    tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock> sig(isolated_ctx, mclient,
                                                                                                "bool_signal", "");

    mclient.connect(mclient.slots[0].name, mclient.signals[0].name, [&](const std::error_code& err) { ut::expect(!err); });

    asio::steady_timer timer{ isolated_ctx };
    timer.expires_from_now(std::chrono::milliseconds(10));
    timer.async_wait([&sig, &test_values](std::error_code) {
      for (auto const value : test_values) {
        sig.async_send(value, [](std::error_code, std::size_t) {});
      }
    });
    isolated_ctx.run_for(std::chrono::seconds(3));
    ut::expect(invocation == test_values.size());
  };
  "Test ipc communication connection and disconnection with mocking int"_test = []() {
    asio::io_context isolated_ctx{};

    tfc::ipc_ruler::ipc_manager_client_mock mclient;

    uint8_t invocation{};
    bool ignore_first{ true };
    std::array<std::int64_t, 3> test_values{ 25, 1337, 42 };

    tfc::ipc::slot<tfc::ipc::details::type_int, tfc::ipc_ruler::ipc_manager_client_mock> slot(
        isolated_ctx, mclient, "bool_slot", "", [&](int value) {
          if (ignore_first) {
            ignore_first = false;
            return;
          }
          ut::expect(test_values.at(invocation++) == value);
          if (invocation == test_values.size()) {
            isolated_ctx.stop();
          }
        });
    tfc::ipc::signal<tfc::ipc::details::type_int, tfc::ipc_ruler::ipc_manager_client_mock> sig(isolated_ctx, mclient,
                                                                                               "bool_signal", "");

    mclient.connect(mclient.slots[0].name, mclient.signals[0].name, [](const std::error_code& err) { ut::expect(!err); });

    asio::steady_timer timer{ isolated_ctx };
    timer.expires_from_now(std::chrono::milliseconds(10));
    timer.async_wait([&sig, &test_values](std::error_code) {
      for (auto const value : test_values) {
        sig.async_send(value, [](std::error_code, std::size_t) {});
      }
    });
    isolated_ctx.run_for(std::chrono::seconds(3));
    ut::expect(invocation == test_values.size());
  };

  return EXIT_SUCCESS;
}
