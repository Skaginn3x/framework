#include <async_mqtt/all.hpp>
#include <boost/program_options.hpp>
#include <tfc/ipc.hpp>

#include "mqtt_broadcaster.hpp"

auto main(int argc, char* argv[]) -> int {
  auto program_description{ tfc::base::default_description() };

  tfc::base::init(argc, argv, program_description);

  asio::io_context io_ctx{};

  mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client,
                   async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtts>, tfc::confman::config<config>,
                   network_manager>
      application(io_ctx);

  application.run();

  return 0;
}
