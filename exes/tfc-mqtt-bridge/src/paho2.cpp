#include "paho2.hpp"
#include <boost/program_options.hpp>

namespace asio = boost::asio;

// TODO: if nothing is printed when the program is started, ipc-ruler is not running
auto main(int argc, char* argv[]) -> int {

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

  const tfc::ipc_ruler::ipc_manager_client ipc_client{ ctx };

  const std::string server_address("mqtt://localhost:1883");

  std::cout << "server_address: " << fmt::format("mqtt://{}:{}", mqtt_host, mqtt_port) << "\n";

  // const std::shared_ptr<mqtt::async_client> mqtt_client = std::make_shared<mqtt::async_client>(server_address, "cid1");
  const std::shared_ptr<mqtt::async_client> mqtt_client = std::make_shared<mqtt::async_client>(fmt::format("mqtt://{}:{}", mqtt_host, mqtt_port), "cid1");

  const mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client&, sdbusplus::bus::match::match>
      application(mqtt_client);

  ctx.run();

  return 0;
}
