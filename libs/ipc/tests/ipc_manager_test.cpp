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

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  using namespace boost::ut;

  boost::asio::io_context ctx;

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
                                       [&](std::error_code&) { ran = true; });
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
    ipc_manager_client.register_slot("test_slot", tfc::ipc::details::type_e::_string, [&](std::error_code&) { ran = true; });

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
    ipc_manager_client.register_connection_change_callback([&](std::string_view slot_name, std::string_view signal_name) {
      boost::ut::expect(slot_name == "test_slot");
      boost::ut::expect(signal_name == "test_signal");
      ran = true;
    });
    boost::ut::expect(!ran);
    ipc_manager_client.connect("test_slot", "test_signal", [](std::error_code& err) { boost::ut::expect(!err); });
    ctx.run_for(std::chrono::milliseconds(5));
    boost::ut::expect(ran);
  };
  "Test ipc communication connection and disconnection with mocking"_test = [&]() {
    bool current_value = false;
    tfc::ipc_ruler::ipc_manager_client_mock mclient;
    tfc::ipc::slot<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock> slot(
        ctx, mclient, "bool_slot", [&](bool value) { current_value = value; });
    tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock> sig(ctx, mclient, "bool_signal");

    mclient.connect(mclient.slots[0].name,
                    mclient.signals[0].name,
                    [](const std::error_code& err) { boost::ut::expect(!err); });

    boost::ut::expect(!current_value);
    sig.send(true);
    ctx.run_for(std::chrono::milliseconds(5));
    boost::ut::expect(current_value);
    sig.send(false);
    ctx.run_for(std::chrono::milliseconds(5));
    boost::ut::expect(!current_value);
    sig.send(true);
    ctx.run_for(std::chrono::milliseconds(5));
    boost::ut::expect(current_value);
  };

  return EXIT_SUCCESS;
}
