#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>

#include <tfc/progbase.hpp>
#include <tfc/confman/file_storage.hpp>
#include <tfc/ipc/details/dbus_server_iface.hpp>

#include "app_operation_mode.hpp"

namespace asio = boost::asio;
namespace po = boost::program_options;

auto main(int argc, char** argv) -> int {
  auto description{ tfc::base::default_description() };
  bool in_memory = false;
  description.add_options()("memory", po::bool_switch(&in_memory), "Run the (ipc-connections) database in memory, non persistent");
  tfc::base::init(argc, argv, description);

  asio::io_context ctx{};

  // Initialize the ipc manager
  auto ipc_manager = std::make_unique<tfc::ipc_ruler::ipc_manager>(in_memory);

  tfc::ipc_ruler::ipc_manager_server const dbus_ipc_server(ctx, std::move(ipc_manager));

  // Initialize operation mode
  [[maybe_unused]] auto const operation_mode{ tfc::app_operation_mode<>(ctx) };

  asio::co_spawn(ctx, tfc::base::exit_signals(ctx), asio::detached);

  ctx.run();

  return EXIT_SUCCESS;
}
