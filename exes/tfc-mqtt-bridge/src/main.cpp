#include <async_mqtt/all.hpp>
#include <tfc/ipc.hpp>
#include "mqtt_broadcaster.hpp"

// TODO: if nothing is printed when the program is started, ipc-ruler is not running
auto main(int argc, char* argv[]) -> int {
  // try {
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

  const std::shared_ptr<async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>> mqtt_client =
      std::make_shared<async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>>(
          async_mqtt::protocol_version::v3_1_1, ctx.get_executor());

  mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client&, sdbusplus::bus::match::match,
                   async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>>
      application(ctx, mqtt_host, mqtt_port, mqtt_username, mqtt_password, ipc_client, mqtt_client);

  ctx.run();

  return 0;
  //} catch (const std::exception& e) {
  //    std::cerr << "Exception caught: " << e.what() << '\n';
  //    return 1;  // or any other non-zero exit code
  //  } catch (...) {
  //    std::cerr << "Unknown exception caught\n";
  //    return 2;  // or any other non-zero exit code
  //  }
}
