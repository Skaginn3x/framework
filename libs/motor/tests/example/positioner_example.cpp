#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>

#include <tfc/progbase.hpp>
#include <tfc/motor/positioner.hpp>
#include <tfc/dbus/string_maker.hpp>

namespace asio = boost::asio;

auto main(int argc, char const* const* argv) -> int {
  tfc::base::init(argc, argv);
  auto ctx{ asio::io_context{} };
  auto dbus{ std::make_shared<sdbusplus::asio::connection>(ctx) };
  dbus->request_name(tfc::dbus::make_dbus_process_name().c_str());

  tfc::motor::positioner::positioner<> pos{ dbus, "foo" };

  ctx.run();

  return 0;
}
