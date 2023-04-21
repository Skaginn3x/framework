#include <filesystem>

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/program_options.hpp>
#include <boost/ut.hpp>
#include <dbus-asio/dbus.h>

#include <tfc/confman.hpp>
#include <tfc/confman/detail/config_rpc_client.hpp>
#include <tfc/confman/detail/config_rpc_server.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/ipc_connector.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;
namespace ut = boost::ut;
using ut::operator""_test;
using ut::operator/;

namespace bpo = boost::program_options;

auto main(int argc, char** argv) -> int {
  auto desc{ tfc::base::default_description() };
  bool run_slow{};
  desc.add_options()("slow,s", bpo::bool_switch(&run_slow)->default_value(false));
  tfc::base::init(argc, argv, desc);

  DBus::Native native{ DBus::Platform::getSessionBus() };
  native.BeginAuth(DBus::AuthenticationProtocol::AUTH_BASIC);


  [[maybe_unused]] ut::suite const rpc_test_cases = [] {
    "happy dbus"_test = [] {

    };
  };
  return static_cast<int>(boost::ut::cfg<>.run({ .report_errors = true }));
}
