#include <iostream>
#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/bus.hpp>

namespace asio = boost::asio;

int main() {
  asio::io_context ctx;
  sd_bus* bus = nullptr;
  if (sd_bus_open_system(&bus) < 0) {
    throw std::runtime_error(std::string{ "Unable to open sd-bus, error: " } + strerror(errno));
  }
  auto connection = std::make_shared<sdbusplus::asio::connection>(ctx, bus);
  auto object_server = sdbusplus::asio::object_server(connection, false);

  connection->request_name("com.testing.app");

  auto dbus_interface = object_server.add_unique_interface("/com/testing/app", "com.testing.app");
  asio::steady_timer timeout(ctx);
  timeout.expires_after(std::chrono::seconds(10));
  timeout.async_wait([&ctx](std::error_code err){
    if (err) return;
    std::cerr << "Timeout, stopping" << std::endl;
    ctx.stop();
  });
  dbus_interface->register_method("ping", [&timeout, &ctx]() -> void {
    std::cout << "ping!" << std::endl;
    timeout.cancel();
    timeout.expires_after(std::chrono::seconds(10));
    timeout.async_wait([&ctx](std::error_code err){
      if (err) return;
      std::cerr << "Timeout, stopping" << std::endl;
      ctx.stop();
    });
    return;
  });

  dbus_interface->initialize();


  asio::steady_timer timer(ctx);
  timer.expires_after(std::chrono::minutes(1));
  timer.async_wait([&](const boost::system::error_code& ec) {
    if (ec) {
      std::cerr << "Error: " << ec.message() << std::endl;
    }
    ctx.stop();
  });

  ctx.run();
  return 0;
}