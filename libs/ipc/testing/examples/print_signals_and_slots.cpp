/**
 * Read signal and slots property from ipc_ruler and print it out
 */
#include <tfc/ipc/details/dbus_client_iface.hpp>

import tfc.base;
import asio;
import fmt;

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  boost::asio::io_context ctx;
  tfc::ipc_ruler::ipc_manager_client man_c(ctx);

  man_c.slots([](auto const& slot_vector) {
    fmt::print("Slots: \n");
    for (auto& slot : slot_vector) {
      fmt::print("{}, ", slot.name);
    }
    fmt::print("\n");
  });

  man_c.signals([](auto const& slot_vector) {
    fmt::print("Signals: \n");
    for (auto const& signal : slot_vector) {
      fmt::print("{}, ", signal.name);
    }
    fmt::print("\n");
  });

  ctx.run_for(std::chrono::seconds(1));
  return EXIT_SUCCESS;
}
