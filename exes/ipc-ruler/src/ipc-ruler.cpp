#include <boost/program_options.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <tfc/confman/file_storage.hpp>
#include <tfc/ipc/details/dbus_server_iface.hpp>

namespace po = boost::program_options;

auto main(int argc, char** argv) -> int {
  auto description{ tfc::base::default_description() };
  bool in_memory = false;
  description.add_options()("memory", po::bool_switch(&in_memory), "Run the database in memory, non persistent");
  tfc::base::init(argc, argv, description);

  boost::asio::io_context ctx;
  auto ipc_manager = std::make_unique<tfc::ipc_ruler::ipc_manager>(in_memory);

  tfc::ipc_ruler::ipc_manager_server const dbus_ipc_server(ctx, std::move(ipc_manager));

  ctx.run();
}
