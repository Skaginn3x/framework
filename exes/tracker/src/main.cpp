
#include <cstdlib>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include <tfc/progbase.hpp>

#include "system.h"

namespace asio = boost::asio;

auto main(int argc, char** argv) -> int {
  auto description{ tfc::base::default_description() };
  tfc::base::init(argc, argv, description);

  asio::io_context ctx{};

  [[maybe_unused]] tfc::tracker::system sys{ ctx };


  return EXIT_SUCCESS;
}
