#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>

#include <tfc/dbus/sd_bus.hpp>

#include <alarm_database.hpp>
#include <dbus_interface.hpp>

namespace po = boost::program_options;
namespace asio = boost::asio;

auto main(int argc, char** argv) -> int {
  auto description{ tfc::base::default_description() };
  bool in_memory = false;
  description.add_options()("memory", po::bool_switch(&in_memory), "Run the database in memory, non persistent");
  tfc::base::init(argc, argv, description);

  asio::io_context ctx;
  auto connection = std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system());

  // Initialize the database
  tfc::themis::alarm_database db(in_memory);

  // Initialize the IPC server
  tfc::themis::interface i(connection, db);

  ctx.run();
  return 0;
}
