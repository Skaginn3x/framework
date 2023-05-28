#include <boost/asio.hpp>

#include <tfc/progbase.hpp>
#include <tfc/stubs/confman.hpp>
#include <tfc/stubs/confman/file_storage.hpp>

namespace asio = boost::asio;

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};

  tfc::confman::stub_config<std::string>{ ctx, "some_key" };

  tfc::confman::stub_file_storage<int>{ ctx, "file_path" };

  // inject to use case

  return EXIT_SUCCESS;
}
