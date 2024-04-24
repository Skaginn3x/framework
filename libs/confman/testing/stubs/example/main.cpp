#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>

#include <tfc/dbus/sd_bus.hpp>
#include <tfc/progbase.hpp>
#include <tfc/stubs/confman.hpp>
#include <tfc/stubs/confman/file_storage.hpp>

namespace asio = boost::asio;

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};
  auto dbus{ std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system()) };

  tfc::confman::stub_config<std::string>{ dbus, "some_key" };

  tfc::confman::stub_file_storage<int>{ ctx, "file_path" };

  // inject to use case

  return EXIT_SUCCESS;
}
