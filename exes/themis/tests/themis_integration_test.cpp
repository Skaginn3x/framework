#include <chrono>

#include <boost/asio.hpp>
#include <boost/ut.hpp>

#include <tfc/dbus/sd_bus.hpp>
#include <tfc/progbase.hpp>
#include <tfc/snitch.hpp>
#include <tfc/snitch/common.hpp>
#include <tfc/snitch/details/dbus_client.hpp>
#include "alarm_database.hpp"
#include "dbus_interface.hpp"

namespace ut = boost::ut;
namespace asio = boost::asio;
using namespace std::chrono_literals;

using boost::ut::operator""_test;
using boost::ut::operator|;
using boost::ut::operator/;
using boost::ut::expect;
using boost::ut::fatal;
using boost::ut::throws;
using tfc::snitch::error;
using tfc::snitch::info;
using tfc::snitch::warning;
using tfc::snitch::warning_latched;
using tfc::snitch::warning_resettable;
using tfc::snitch::detail::dbus_client;
using tfc::themis::alarm_database;
using tfc::themis::interface;

struct test_setup_s {
  asio::io_context ctx;
  alarm_database db;
  std::shared_ptr<sdbusplus::asio::connection> connection;
  interface face;
  std::thread t;
  test_setup_s()
      : db(true), connection{ std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system()) },
        face(connection, db), t([&] { ctx.run(); }) {}
  ~test_setup_s() {
    ctx.stop();
    t.join();
  }
};

struct test_setup_c {
  asio::io_context ctx;
  std::shared_ptr<sdbusplus::asio::connection> connection;
  std::array<bool, 10> ran{};
  std::array<std::uint64_t, 10> ids{};
  dbus_client client;
  test_setup_c()
      : connection{ std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system()) },
        client(connection) {}
  ~test_setup_c() {
    expect(connection.use_count() == 2);
    connection->close();
  }
};

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
        face(sconnection, db), client(connection), t([&] { sctx.run(); }) {}
  ~test_setup() {
    sctx.stop();
    t.join();
  }
};

struct formattable_type {
  constexpr operator std::string_view() const noexcept { return "42"; }
};
constexpr auto format_as(formattable_type const& val) -> std::string_view {
  return std::string_view{ val };
}

int main(int argc, char** argv) {
  tfc::base::init(argc, argv);
  "Test some dbus functions for signature errors and general correctness"_test = [] {
    test_setup t;
    info<"desc", "details"> i(t.connection, "first_test");
    i.set([&](const std::error_code& err) {
      expect(!err) << err.message();
      t.ran[0] = true;
    });
    t.ctx.run_for(100ms);
    expect(t.ran[0]);
    t.client.list_alarms([&](const std::error_code& err, std::vector<tfc::snitch::api::alarm> alarms) {
      expect(!err) << err.message();
      expect(alarms.size() == 1);
      t.ran[5] = true;
    });
    t.ctx.run_for(2ms);
    expect(t.ran[5]);

    t.client.list_activations(
        "en", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::active_e::active,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()),
        [&](const std::error_code& err, std::vector<tfc::snitch::api::activation> act) {
          expect(!err) << err.message();
          expect(act.size() == 1);
          expect(act.at(0).lvl == tfc::snitch::level_e::info);
          expect(!act.at(0).reset_timestamp.has_value());
          t.ran[1] = true;
        });
    t.ctx.run_for(2ms);
    expect(t.ran[1]);
    i.reset([&](const std::error_code& err) {
      expect(!err) << err.message();
      t.ran[2] = true;
    });
    t.ctx.run_for(2ms);
    expect(t.ran[2]);
    t.client.list_activations(
        "en", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::active_e::active,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()),
        [&](const std::error_code& err, std::vector<tfc::snitch::api::activation> act) {
          expect(!err) << err.message();
          expect(act.size() == 0);
          t.ran[3] = true;
        });
    t.client.list_activations(
        "en", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::active_e::inactive,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()),
        [&](const std::error_code& err, std::vector<tfc::snitch::api::activation> act) {
          expect(!err) << err.message();
          expect(act.size() == 1);
          expect(act.at(0).reset_timestamp.has_value());
          t.ran[4] = true;
        });
    t.ctx.run_for(3ms);
    expect(t.ran[3]);
    expect(t.ran[4]);
  };
  "Test if the try_reset signal makes it in this world"_test = [] {
    test_setup t;
    warning_latched<"desc", "details"> w(t.connection, "first_test");
    t.ctx.run_for(1ms);
    w.on_try_reset([&]() { t.ran[1] = true; });
    t.client.try_reset_alarm(w.alarm_id().value(), [&](const std::error_code& err) {
      expect(!!err);
      t.ran[2] = true;
    });
    t.ctx.run_for(2ms);
    expect(!t.ran[1]);
    expect(t.ran[2]);

    w.set([&](const std::error_code& err) {
      expect(!err) << err.message();
      t.ran[0] = true;
    });
    t.ctx.run_for(2ms);
    expect(t.ran[0]);
    expect(w.alarm_id().has_value());
    w.on_try_reset([&]() { t.ran[3] = true; });
    t.client.try_reset_alarm(w.alarm_id().value(), [&](const std::error_code& err) {
      expect(!err) << err.message();
      t.ran[4] = true;
    });
    t.ctx.run_for(2ms);
    expect(t.ran[3]);
    expect(t.ran[4]);
  };
  "Test is the try_reset_all signal tickles these fancies"_test = [] {
    test_setup t;
    warning_latched<"desc1", "details"> wl(t.connection, "first_test");
    warning_resettable<"desc2", "details"> wr(t.connection, "first_test");
    error<"desc3", "details"> e(t.connection, "first_test");
    wl.on_try_reset([&]() { t.ran[1] = true; });
    wr.on_try_reset([&]() { t.ran[2] = true; });
    e.on_try_reset([&]() { t.ran[3] = true; });
    t.client.try_reset_all_alarms([&](const std::error_code& err) {
      expect(!!err);
      t.ran[0] = true;
    });
    t.ctx.run_for(2ms);
    expect(t.ran[0]);
    expect(!t.ran[1]);
    expect(!t.ran[2]);
    expect(!t.ran[3]);

    wl.set([&](const std::error_code& err) {
      expect(!err) << err.message();
      t.ran[4] = true;
    });
    t.client.try_reset_all_alarms([&](const std::error_code& err) {
      expect(!err) << err.message();
      t.ran[5] = true;
    });
    t.ctx.run_for(2ms);
    expect(t.ran[4]);
    expect(t.ran[5]);
    expect(t.ran[1]);

    t.ran[1] = false;
    wr.set([&](const std::error_code& err) {
      expect(!err) << err.message();
      t.ran[6] = true;
    });
    e.set([&](const std::error_code& err) {
      expect(!err) << err.message();
      t.ran[7] = true;
    });
    t.client.try_reset_all_alarms([&](const std::error_code& err) {
      expect(!err) << err.message();
      t.ran[8] = true;
    });
    t.ctx.run_for(2ms);
    expect(t.ran[6]);
    expect(t.ran[7]);
    expect(t.ran[8]);
    expect(t.ran[1]);
    expect(t.ran[2]);
    expect(t.ran[3]);
  };

  "alarm with declared params but no params provided"_test = [] {
    test_setup t;
    info<"desc {foo}", "details {bar} {foo}"> i(t.connection, "first_test");
    expect(throws([&] { i.set(); }));
  };

  "alarm with wrong named params"_test = [] {
    test_setup t;
    info<"desc {foo}", "details {bar} {foo}"> i(t.connection, "first_test");
    expect(throws([&] { i.set(fmt::arg("ababa", 1), fmt::arg("some_other name", 2)); }));
  };

  "alarm with params"_test = [] {
    test_setup t;
    info<"desc {foo}", "details {bar} {foo}"> i(t.connection, "first_test");
    i.set(
        [&](auto err) {
          expect(!err) << fmt::format("Received error: {}", err.message());
          t.ran[0] = true;

          t.client.list_activations(
              "en", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::active_e::active,
              tfc::themis::alarm_database::timepoint_from_milliseconds(0),
              tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()),
              [&](auto list_err, std::vector<tfc::snitch::api::activation> const& act) {
                expect(!list_err) << list_err.message();
                expect(fatal(act.size() == 1));
                auto const& alarm = act.at(0);
                expect(alarm.description == "desc 1337");
                expect(alarm.details == "details 42 1337");
                t.ran[1] = true;
              });
        },
        fmt::arg("foo", 1337), fmt::arg("bar", formattable_type{}));
    t.ctx.run_for(2ms);
    expect(t.ran[0]);
    expect(t.ran[1]);
  };

  // TODO: The alarm is recreated but its state is not set again.
  ut::skip / "Alarm loses database connection set forgotten"_test = [] {
    test_setup_s* server = new test_setup_s();
    test_setup_c client;
    info<"desc", "details"> i(client.connection, "dead_server_test");
    {
      i.set([&](auto err) {
        expect(!err) << fmt::format("Received error: {}", err.message());
        client.ran[0] = true;
      });
      client.ctx.run_for(2ms);
      expect(client.ran[0]);
      delete server;
    }
    {
      // This servers database is started in ram again.
      // should the alarm should be populated and set again.
      client.ran[0] = false;  // Reset the set callback. Should probably be called again.
      client.ctx.run_for(2ms);
      test_setup_s* server2 = new test_setup_s();
      client.ctx.run_for(2ms);
      expect(client.ran[0]);
      client.client.list_alarms([&](const std::error_code& err, std::vector<tfc::snitch::api::alarm> alarms) {
        expect(!err) << err.message();
        expect(alarms.size() == 1);
        client.ran[1] = true;
      });
      client.ctx.run_for(2ms);
      expect(client.ran[1]);
      client.client.list_activations(
          "en", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::active_e::active,
          tfc::themis::alarm_database::timepoint_from_milliseconds(0),
          tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()),
          [&](const std::error_code& err, std::vector<tfc::snitch::api::activation> act) {
            expect(!err) << err.message();
            expect(act.size() == 1);
            client.ran[2] = true;
          });
      client.ctx.run_for(2ms);
      expect(client.ran[2]);
      delete server2;
    }
  };

  // TODO: An extra alarm could be created that warns of the lost connection.
  "An alarm that is lost shall have its activations status set to unknown"_test = [] {
    test_setup_s server;
    test_setup_c* client = new test_setup_c();
    info<"desc", "details">* i = new info<"desc", "details">(client->connection, "dead_client_test");
    client->connection->request_name("com.skaginn3x.test_this_name");
    client->client.list_alarms([&](const std::error_code& err, std::vector<tfc::snitch::api::alarm> alarms) {
      expect(!err) << err.message();
      expect(alarms.size() == 1);
      client->ran[0] = true;
    });
    client->ctx.run_for(2ms);
    expect(client->ran[0]);
    i->set([&](auto err) {
      expect(!err) << fmt::format("Received error: {}", err.message());
      client->ran[1] = true;
    });
    client->ctx.run_for(2ms);
    expect(client->ran[1]);
    delete i;
    delete client;
    fmt::println(stderr, "Client has been killed");
    test_setup_c new_client;
    new_client.client.list_activations(
        "en", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::active_e::unknown,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()),
        [&](const std::error_code& err, std::vector<tfc::snitch::api::activation> act) {
          expect(!err) << err.message();
          expect(act.size() == 1);
          auto const& alarm = act.at(0);
          expect(alarm.description == "desc");
          expect(alarm.details == "details");
          expect(alarm.lvl == tfc::snitch::level_e::info);
          expect(alarm.active == tfc::snitch::api::active_e::unknown);
          new_client.ran[0] = true;
        });
    new_client.ctx.run_for(2ms);
    expect(new_client.ran[0]);
  };
  return 0;
}
