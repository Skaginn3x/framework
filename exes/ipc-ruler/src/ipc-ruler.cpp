#include <sdbusplus/asio/connection.hpp>
#include "tfc/ipc_connector/dbus_server_iface.hpp"

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  boost::asio::io_context ctx;

  tfc::ipc_ruler::ipc_manager_server const dbus_ipc_server(ctx);

  ctx.run();
}
