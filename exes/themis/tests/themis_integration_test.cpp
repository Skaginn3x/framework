#include <alarm_database.hpp>
#include <boost/asio.hpp>
#include <boost/ut.hpp>
#include <chrono>
#include <dbus_interface.hpp>
#include <iostream>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/progbase.hpp>
#include <tfc/snitch.hpp>
#include <tfc/snitch/common.hpp>
#include <tfc/snitch/details/dbus_client.hpp>

namespace ut = boost::ut;
namespace asio = boost::asio;
using namespace std::chrono_literals;

using boost::ut::operator""_test;
using boost::ut::operator|;
using boost::ut::operator/;
using boost::ut::expect;
using boost::ut::throws;
using tfc::snitch::error;
using tfc::snitch::info;
using tfc::snitch::warning;
using tfc::snitch::detail::dbus_client;
using tfc::themis::alarm_database;
using tfc::themis::interface;

struct test_setup {
  asio::io_context ctx;
  asio::io_context sctx;
  alarm_database db;
  std::shared_ptr<sdbusplus::asio::connection> sconnection;
  std::shared_ptr<sdbusplus::asio::connection> connection;
  interface face;
  std::array<bool, 10> ran{};
  std::array<std::uint64_t, 10> ids{};
  dbus_client client;
  std::thread t;
  test_setup()
      : db(true), sconnection{ std::make_shared<sdbusplus::asio::connection>(sctx, tfc::dbus::sd_bus_open_system()) },
        connection{ std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system()) },
        face(connection, db), client(connection), t([&]{
          sctx.run();
        }){}
  ~test_setup() {
      sctx.stop();
      t.join();
  }
};

int main(int argc, char** argv) {
  tfc::base::init(argc, argv);
  "Basics working"_test = [] {
    test_setup t;
    info<"desc", "details"> i(t.connection, "first_test");
    i.set([&](const std::error_code& err) {
      expect(!err) << err.message();
      t.ran[0] = true;
    });
    t.ctx.run_for(2ms);
    expect(t.ran[0]);
    // expect(t.db.list_alarms().size() == 1);

    t.client.list_activations(
        "en", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::active_e::active,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()),
        [&](const std::error_code& err, std::vector<tfc::snitch::api::activation> act) {
          expect(!err) << err.message();
          expect(act.size() == 1);
          expect(act.at(0).lvl == tfc::snitch::level_e::info);
          t.ran[1] = true;
        });
    t.ctx.run_for(2ms);
    expect(t.ran[1]);
    i.reset();
  };
  return 0;
}
