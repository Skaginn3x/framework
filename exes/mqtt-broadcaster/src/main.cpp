#include <async_mqtt/all.hpp>
#include <boost/program_options.hpp>
#include <tfc/ipc.hpp>
#include "mqtt_broadcaster.hpp"
#include "config.hpp"
#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>

auto main(int argc, char* argv[]) -> int {
  auto program_description{ tfc::base::default_description() };

  tfc::base::init(argc, argv, program_description);

  asio::io_context io_ctx{};

  const mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client, config, tfc::confman::config>
      application(io_ctx);

  io_ctx.run();

  return 0;
}
