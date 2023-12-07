#include <iostream>
#include <tfc/progbase.hpp>
#include "dbus_screen_manager.hpp"

int main(int argc, char** argv) {
  auto description{ tfc::base::default_description() };

  std::string signal{};
  std::string slot{};
  std::vector<std::string> connect;
  bool list_signals{};
  bool list_slots{};

  description.add_options()("signal", po::value<std::string>(&signal), "IPC signal channel (output)")(
      "slot", po::value<std::string>(&slot), "IPC slot channel (input)")(
      "connect,c", po::value<std::vector<std::string>>(&connect)->multitoken(), "Listen to these slots")(
      "list-signals", po::bool_switch(&list_signals), "List all available IPC signals")(
      "list-slots", po::bool_switch(&list_slots), "List all available IPC slots");
  tfc::base::init(argc, argv, description);
  try {
    DBusScreenManager sm;
    sm.Run();
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
