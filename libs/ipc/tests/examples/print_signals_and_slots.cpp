/**
 * Read signal and slots property from ipc_ruler and print it out
 */
#include <fmt/format.h>
#include <iostream>
#include <tfc/ipc/details/dbus_server_iface.hpp>
#include <tfc/progbase.hpp>

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  boost::asio::io_context ctx;
  tfc::ipc_ruler::ipc_manager_client man_c(ctx);

  man_c.slots([](auto v) {
    std::cout << "Slots: \n";
    for (auto& sl : v) {
      std::cout << sl.name << std::endl;
    }
  });

  man_c.signals([](auto v) {
    std::cout << "Signals: \n";
    for (auto& sl : v) {
      std::cout << sl.name << std::endl;
    }
  });

  ctx.run_for(std::chrono::seconds(1));
  return EXIT_SUCCESS;
}
