#include <stdlib.h>
#include <string>

#include <tfc/confman/detail/config_rpc_server.hpp>
#include <tfc/progbase.hpp>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

namespace bpo = boost::program_options;
namespace asio = boost::asio;

auto main(int argc, char** argv) -> int {
  auto desc{ tfc::base::default_description() };
  std::string file_name{};
  desc.add_options()("file,f", bpo::value<std::string>(&file_name), "Name of configuration file.");
  tfc::base::init(argc, argv, desc);

  asio::io_context ctx{};

  tfc::confman::detail::config_rpc_server services_server{ ctx, file_name };

  ctx.run();
  return EXIT_SUCCESS;
}
