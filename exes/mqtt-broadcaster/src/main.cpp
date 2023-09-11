#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <tfc/ipc.hpp>

#include <impl/network_manager_ssl.hpp>

#include "broadcaster.hpp"

namespace asio = boost::asio;

auto main(int argc, char* argv[]) -> int {
  auto program_description{ tfc::base::default_description() };

  tfc::base::init(argc, argv, program_description);

  asio::io_context io_ctx{};

  tfc::mqtt::broadcaster<tfc::ipc_ruler::ipc_manager_client, tfc::confman::config<tfc::mqtt::config>,
                         tfc::mqtt::impl::network_manager_ssl>
      application(io_ctx);

  application.run();

  return 0;
}
