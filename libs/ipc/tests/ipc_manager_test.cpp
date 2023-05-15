#include <boost/asio.hpp>
#include <boost/ut.hpp>
#include <glaze/glaze.hpp>
#include <tfc/confman/file_storage.hpp>
#include <tfc/ipc/details/dbus_server_iface.hpp>
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

  auto ipc_manager = tfc::ipc_ruler::ipc_manager<signal_storage, slot_storage>(signals, slots);

  "ipc_manager correctness check"_test = [&]() {
    ipc_manager.register_signal("some_slot", tfc::ipc::details::type_e::_bool);
    boost::ut::expect(ipc_manager.get_all_signals().size() == 1);
    boost::ut::expect(ipc_manager.get_all_slots().empty());

    signals.storage_ = {};
    boost::ut::expect(ipc_manager.get_all_signals().empty());
  };

  return EXIT_SUCCESS;
}
