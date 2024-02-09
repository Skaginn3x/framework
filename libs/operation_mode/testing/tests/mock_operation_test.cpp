#include <boost/ut.hpp>
#include <boost/asio/io_context.hpp>
#include <tfc/progbase.hpp>
#include <tfc/mocks/operation_mode.hpp>


using boost::ut::operator""_test;
using std::chrono::operator""ms;
using boost::ut::expect;

namespace asio = boost::asio;

struct instance {
  asio::io_context ctx;
  tfc::operation::mock_interface op{ ctx };
  std::array<bool, 10> ran;
};

int main(int, char** argv) {
  std::array<char const*, 4> args{ argv[0], "--log-level", "trace", "--stdout" };
  tfc::base::init(args.size(), args.data());
  "basic mode test"_test = [] {
    instance i;
    i.op.on_enter(tfc::operation::mode_e::running, [&i](tfc::operation::mode_e, tfc::operation::mode_e) {
      i.ran[0] = true;
    });
    i.op.set(tfc::operation::mode_e::running);
    i.op.stop("test");
    expect(i.ran[0]);
  };
}
