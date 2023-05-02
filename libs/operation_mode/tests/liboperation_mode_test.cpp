
#include <boost/ut.hpp>
#include <boost/asio.hpp>
#include <tfc/progbase.hpp>
#include <tfc/operation_mode.hpp>

#include <sdbusplus/asio/connection.hpp>

namespace asio = boost::asio;

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  using boost::ut::operator""_test;
  using boost::ut::expect;

  asio::io_context ctx{};

//  [[maybe_unused]] sdbusplus::asio::connection foo(ctx);
//  [[maybe_unused]] sdbusplus::asio::connection f(ctx);

  tfc::operation::interface mode{ctx};

  mode.on_leave(tfc::operation::mode_e::stopped, [](tfc::operation::mode_e, tfc::operation::mode_e){});

  ctx.run();

  return static_cast<int>(boost::ut::cfg<>.run({ .report_errors = true }));
}
