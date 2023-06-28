#include "paho.hpp"
#include <mqtt/async_client.h>
#include <tfc/ipc.hpp>

// TODO: if nothing is printed when the program is started, ipc-ruler is not running
auto main(int argc, char* argv[]) -> int {

  std::string persist_dir_ = "./persist";

  auto program_description{ tfc::base::default_description() };

  std::string mqtt_host;
  std::string mqtt_port;
  std::string mqtt_username;
  std::string mqtt_password;

  program_description.add_options()("mqtt_host", boost::program_options::value<std::string>(&mqtt_host)->required(),
                                    "ip address of mqtt broker")(
      "mqtt_port", boost::program_options::value<std::string>(&mqtt_port)->required(), "port of mqtt broker")(
      "mqtt_username", boost::program_options::value<std::string>(&mqtt_username), "username of mqtt broker")(
      "mqtt_password", boost::program_options::value<std::string>(&mqtt_password), "password of mqtt broker");

  tfc::base::init(argc, argv, program_description);

  asio::io_context ctx{};

  tfc::ipc_ruler::ipc_manager_client ipc_client{ ctx };

  auto mqtt_client = std::make_shared<mqtt::async_client>(mqtt_host, "cid1", persist_dir_);

  // TODO: test this using ipc-ruler and gets this baby going
  mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client&, sdbusplus::bus::match::match>
      application(ctx, mqtt_host, mqtt_port, mqtt_username, mqtt_password, ipc_client, mqtt_client);

  ctx.run();

  return 0;
}
