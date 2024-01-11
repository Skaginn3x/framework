#include <boost/asio.hpp>
#include <boost/ut.hpp>

#include <tfc/ec/devices/schneider/atv320/dbus-iface.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;
namespace ut = boost::ut;

using ut::operator""_test;
using ut::operator|;
using ut::expect;

auto main(int argc, char const* const* argv) -> int {
  return EXIT_SUCCESS;
}
