#include <boost/asio.hpp>
#include <boost/ut.hpp>

#include <tfc/operation_mode.hpp>
#include <tfc/progbase.hpp>
#include "app_operation_mode.hpp"

namespace asio = boost::asio;

using boost::ut::operator""_test;

struct operation_mode_test {
  asio::io_context ctx{};
  tfc::app_operation_mode app{ ctx };
  tfc::operation::interface lib {
    ctx
  };
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  "set mode"_test = [] {
    operation_mode_test test{};
    test.lib.set(tfc::operation::mode_e::starting);
    test.ctx.run_for(std::chrono::milliseconds(100));
  };

  return EXIT_SUCCESS;
}
