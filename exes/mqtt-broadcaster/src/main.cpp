#include <tfc/ipc.hpp>
#include "mqtt_broadcaster.hpp"
#include <async_mqtt/all.hpp>


// TODO: if nothing is printed when the program is started, ipc-ruler is not running
int main(int argc, char* argv[]) {
  auto prog_desc{ tfc::base::default_description() };

  std::string mqtt_host, mqtt_port;

  prog_desc.add_options()("mqtt_host", boost::program_options::value<std::string>(&mqtt_host)->required(),
                          "ip address of mqtt broker")(
      "mqtt_port", boost::program_options::value<std::string>(&mqtt_port)->required(), "port of mqtt broker");

  tfc::base::init(argc, argv, prog_desc);

  asio::io_context ctx{};

  tfc::ipc_ruler::ipc_manager_client ipc_client{ ctx };

  mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client&, sdbusplus::bus::match::match, async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>>
      application(ctx, mqtt_host, mqtt_port, ipc_client);

  ctx.run();

  return 0;
}
