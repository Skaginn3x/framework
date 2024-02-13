#include <sdbusplus/asio/connection.hpp>
#include <tfc/confman/file_storage.hpp>
#include <tfc/ipc/details/dbus_server_iface.hpp>

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  boost::asio::io_context ctx;
  auto ipc_manager = std::make_unique<tfc::ipc_ruler::ipc_manager>();

  tfc::ipc_ruler::ipc_manager_server const dbus_ipc_server(ctx, std::move(ipc_manager));

  tfc::base::run(ctx);
}
