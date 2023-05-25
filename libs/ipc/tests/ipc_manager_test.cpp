#include <boost/asio.hpp>
#include <boost/ut.hpp>
#include <glaze/glaze.hpp>
#include <tfc/confman/file_storage.hpp>
#include <tfc/ipc.hpp>
#include <tfc/progbase.hpp>

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

struct ctx_runner {
  ctx_runner(boost::asio::io_context& ctx) : ctx_{ ctx } {}

  void run_while(std::chrono::nanoseconds max_run_time) {
    run = true;
    auto start = std::chrono::high_resolution_clock::now();
    while (run && max_run_time > std::chrono::high_resolution_clock::now() - start) {
      ctx_.run_one();
    }
    boost::ut::expect(!run);
  }

  boost::asio::io_context& ctx_;
  bool run = true;
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  using namespace boost::ut;

  boost::asio::io_context ctx;
  ctx_runner runner(ctx);

  using signal_storage = file_storage_mock<std::unordered_map<std::string, tfc::ipc_ruler::signal>>;
  using slot_storage = file_storage_mock<std::unordered_map<std::string, tfc::ipc_ruler::slot>>;

  auto signals = signal_storage();
  auto slots = slot_storage();

  auto ipc_manager = std::make_unique<tfc::ipc_ruler::ipc_manager<signal_storage, slot_storage>>(signals, slots);

  "ipc_manager correctness check"_test = [&]() {
    ipc_manager->register_signal("some_slot", tfc::ipc::details::type_e::_bool);
    boost::ut::expect(ipc_manager->get_all_signals().size() == 1);
    boost::ut::expect(ipc_manager->get_all_slots().empty());

    signals.storage_ = {};
    boost::ut::expect(ipc_manager->get_all_signals().empty());
  };

  auto ipc_manager_server = tfc::ipc_ruler::ipc_manager_server<signal_storage, slot_storage>(ctx, std::move(ipc_manager));

  auto ipc_manager_client = tfc::ipc_ruler::ipc_manager_client(ctx);

  "ipc_manager check over dbus"_test = [&]() {
    // Check if the correct empty list is reported for signals
    bool ran = false;
    ipc_manager_client.signals([&](auto v) {
      boost::ut::expect(v.empty());
      ran = true;
    });
    ctx.run_for(std::chrono::milliseconds(5));
    boost::ut::expect(ran);

    // Check if the correct empty list is reported for slots
    ipc_manager_client.slots([&](auto v) {
      boost::ut::expect(v.empty());
      ran = true;
    });
    ctx.run_for(std::chrono::milliseconds(5));
    boost::ut::expect(ran);

    // add a signal
    ran = false;
    ipc_manager_client.register_signal("test_signal", tfc::ipc::details::type_e::_string,
                                       [&](const std::error_code&) { ran = true; });
    ctx.run_for(std::chrono::milliseconds(5));
    boost::ut::expect(ran);

    // check that the signal appears
    ran = false;
    ipc_manager_client.signals([&](auto v) {
      boost::ut::expect(!v.empty());
      boost::ut::expect(v.size() == 1);
      boost::ut::expect(v[0].name == "test_signal");
      ran = true;
    });
    ctx.run_for(std::chrono::milliseconds(5));
    boost::ut::expect(ran);

    // Register a slot
    ran = false;
    ipc_manager_client.register_slot("test_slot", tfc::ipc::details::type_e::_string, [&](const std::error_code& err) {
      boost::ut::expect(!err);
      ran = true;
    });

    // Check that the slot appears
    ctx.run_for(std::chrono::milliseconds(5));
    boost::ut::expect(ran);
    ran = false;
    ipc_manager_client.slots([&](auto v) {
      boost::ut::expect(!v.empty());
      boost::ut::expect(v.size() == 1);
      boost::ut::expect(v[0].name == "test_slot");
      ran = true;
    });
    ctx.run_for(std::chrono::milliseconds(5));
    boost::ut::expect(ran);

    // Register a method for connection change callback
    ran = false;
    ipc_manager_client.register_connection_change_callback("test_slot", [&](std::string_view signal_name) {
      boost::ut::expect(signal_name == "test_signal");
      ran = true;
    });
    boost::ut::expect(!ran);
    ipc_manager_client.connect("test_slot", "test_signal", [](const std::error_code& err) { boost::ut::expect(!err); });
    ctx.run_for(std::chrono::milliseconds(5));
    boost::ut::expect(ran);
  };
  "Check that re-registering a communication channel only changes last_registered "_test = [&]() {
    // Check if the correct empty list is reported for signals

    bool registered_signal = false;
    bool registered_slot = false;
    // Register a signal and slot for the first time
    ipc_manager_client.register_signal("signal_register_retest", tfc::ipc::details::type_e::_string,
                                       [&](const std::error_code& err) {
                                         boost::ut::expect(!err);
                                         registered_signal = true;
                                       });
    // Expect to get a callback
    bool got_callback = false;
    ipc_manager_client.register_connection_change_callback("slot_register_retest",
                                                           [&](const std::string_view&) { got_callback = true; });
    ipc_manager_client.register_slot("slot_register_retest", tfc::ipc::details::type_e::_string,
                                     [&](const std::error_code& err) {
                                       boost::ut::expect(!err);
                                       registered_slot = true;
                                     });
    ctx.run_for(std::chrono::milliseconds(5));
    boost::ut::expect(got_callback);
    boost::ut::expect(registered_signal);
    boost::ut::expect(registered_slot);

    // Connect the slot to the signal
    ipc_manager_client.connect("slot_register_retest", "signal_register_retest",
                               [](const std::error_code& err) { boost::ut::expect(!err); });

    ctx.run_for(std::chrono::milliseconds(5));

    tfc::ipc_ruler::signal signal_copy;
    tfc::ipc_ruler::slot slot_copy;
    // Copy the registered slot and signal and then re-register and verify the information is intact
    ipc_manager_client.signals([&](auto v) {
      for (auto& signal : v) {
        if (signal.name == "signal_register_retest") {
          signal_copy = signal;
          return;
        }
      }
      boost::ut::expect(false);
    });
    ctx.run_for(std::chrono::milliseconds(5));
    ipc_manager_client.slots([&](auto v) {
      for (auto& slot : v) {
        if (slot.name == "slot_register_retest") {
          slot_copy = slot;
          return;
        }
      }
      boost::ut::expect(false);
    });
    ctx.run_for(std::chrono::milliseconds(5));
    boost::ut::expect(!signal_copy.name.empty());
    boost::ut::expect(!slot_copy.name.empty());

    registered_signal = false;
    registered_slot = false;
    // Register a signal and slot for the second time
    ipc_manager_client.register_signal("signal_register_retest", tfc::ipc::details::type_e::_string,
                                       [&](const std::error_code& err) {
                                         boost::ut::expect(!err);
                                         registered_signal = true;
                                       });
    ipc_manager_client.register_slot("slot_register_retest", tfc::ipc::details::type_e::_string,
                                     [&](const std::error_code& err) {
                                       boost::ut::expect(!err);
                                       registered_slot = true;
                                     });
    ctx.run_for(std::chrono::milliseconds(5));
    boost::ut::expect(registered_signal);
    boost::ut::expect(registered_slot);

    tfc::ipc_ruler::signal signal_copy2;
    tfc::ipc_ruler::slot slot_copy2;
    // Copy the registered slot and signal and then re-register and verify the information is intact
    ipc_manager_client.signals([&](auto v) {
      for (auto& signal : v) {
        if (signal.name == "signal_register_retest") {
          signal_copy2 = signal;
          return;
        }
      }
      boost::ut::expect(false);
    });
    ctx.run_for(std::chrono::milliseconds(5));
    ipc_manager_client.slots([&](auto v) {
      for (auto& slot : v) {
        if (slot.name == "slot_register_retest") {
          slot_copy2 = slot;
          return;
        }
      }
      boost::ut::expect(false);
    });
    ctx.run_for(std::chrono::milliseconds(5));
    boost::ut::expect(!signal_copy2.name.empty());
    boost::ut::expect(!slot_copy2.name.empty());

    // Compare the two copies and verify that only the last_registered value changed
    boost::ut::expect(signal_copy.name == signal_copy2.name);
    boost::ut::expect(signal_copy.created_at == signal_copy2.created_at);
    boost::ut::expect(signal_copy.created_by == signal_copy2.created_by);
    boost::ut::expect(signal_copy.type == signal_copy2.type);
    boost::ut::expect(signal_copy.last_registered != signal_copy2.last_registered);

    boost::ut::expect(slot_copy.name == slot_copy2.name);
    boost::ut::expect(slot_copy.created_at == slot_copy2.created_at);
    boost::ut::expect(slot_copy.created_by == slot_copy2.created_by);
    boost::ut::expect(slot_copy.type == slot_copy2.type);
    boost::ut::expect(slot_copy.modified_by == slot_copy2.modified_by);
    boost::ut::expect(slot_copy.connected_to == slot_copy2.connected_to);
    boost::ut::expect(slot_copy.last_registered != slot_copy2.last_registered);
  };
  "Test ipc communication connection and disconnection with mocking bool"_test = [&]() {
    bool current_value = false;
    tfc::ipc_ruler::ipc_manager_client_mock mclient;
    tfc::ipc::slot<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock> slot(ctx, mclient, "bool_slot",
                                                                                               [&](bool value) {
                                                                                                 current_value = value;
                                                                                                 runner.run = false;
                                                                                               });
    tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock> sig(ctx, mclient, "bool_signal");

    mclient.connect(mclient.slots[0].name, mclient.signals[0].name,
                    [&](const std::error_code& err) { boost::ut::expect(!err); });

    boost::ut::expect(!current_value);
    sig.send(true);
    runner.run_while(std::chrono::seconds(1));
    boost::ut::expect(current_value);
    sig.send(false);
    runner.run_while(std::chrono::seconds(1));
    boost::ut::expect(!current_value);
    sig.send(true);
    runner.run_while(std::chrono::seconds(1));
    boost::ut::expect(current_value);
  };
  "Test ipc communication connection and disconnection with mocking int"_test = [&]() {
    int current_value = 10;
    tfc::ipc_ruler::ipc_manager_client_mock mclient;
    tfc::ipc::slot<tfc::ipc::details::type_int, tfc::ipc_ruler::ipc_manager_client_mock> slot(ctx, mclient, "bool_slot",
                                                                                              [&](int value) {
                                                                                                current_value = value;
                                                                                                runner.run = false;
                                                                                              });
    tfc::ipc::signal<tfc::ipc::details::type_int, tfc::ipc_ruler::ipc_manager_client_mock> sig(ctx, mclient, "bool_signal");

    mclient.connect(mclient.slots[0].name, mclient.signals[0].name,
                    [](const std::error_code& err) { boost::ut::expect(!err); });

    boost::ut::expect(current_value == 10);
    sig.send(25);
    runner.run_while(std::chrono::seconds(1));
    boost::ut::expect(current_value == 25);
    sig.send(1337);
    runner.run_while(std::chrono::seconds(1));
    boost::ut::expect(current_value == 1337);
    sig.send(42);
    runner.run_while(std::chrono::seconds(1));
    boost::ut::expect(current_value == 42);
  };

  return EXIT_SUCCESS;
}
