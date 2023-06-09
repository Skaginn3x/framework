#include <sdbusplus/asio/connection.hpp>
#include <tfc/confman/file_storage.hpp>
#include <tfc/ipc/details/dbus_server_iface.hpp>

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  boost::asio::io_context ctx;

  using signal_storage = tfc::confman::file_storage<std::map<std::string, tfc::ipc_ruler::signal>>;
  using slot_storage = tfc::confman::file_storage<std::map<std::string, tfc::ipc_ruler::slot>>;

  auto signals = signal_storage(ctx, tfc::base::make_config_file_name("signal", "json"));
  auto slots = slot_storage(ctx, tfc::base::make_config_file_name("slots", "json"));

  auto ipc_manager = std::make_unique<tfc::ipc_ruler::ipc_manager<signal_storage, slot_storage>>(signals, slots);

  tfc::ipc_ruler::ipc_manager_server<signal_storage, slot_storage> const dbus_ipc_server(ctx, std::move(ipc_manager));

  ctx.run();
}
