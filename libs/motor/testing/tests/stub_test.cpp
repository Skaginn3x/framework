#include <mp-units/systems/si/si.h>
#include <boost/asio.hpp>
#include <boost/ut.hpp>
#include <iostream>
#include <tfc/motor.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;

using micrometre_t = tfc::motor::dbus::types::micrometre_t;
using speedratio_t = tfc::motor::dbus::types::speedratio_t;
using boost::ut::operator""_test;
using std::chrono::operator""ms;
using boost::ut::expect;
using namespace mp_units::si::unit_symbols;

using api = tfc::motor::api;

struct instance {
  asio::io_context ctx;
  std::shared_ptr<sdbusplus::asio::connection> conn =
      std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system());
  std::error_code res;
  micrometre_t length;
  std::array<bool, 10> ran{};
};

int main(int, char** argv) {
  std::array<char const*, 4> args{ argv[0], "--log-level", "trace", "--stdout" };
  tfc::base::init(args.size(), args.data());
  using enum tfc::motor::errors::err_enum;
  using tfc::motor::motor_error;

  "Stub test run"_test = [&] {
    instance i;
    std::shared_ptr<tfc::motor::api::config_t> motor_conf =
        std::make_shared<tfc::motor::api::config_t>(tfc::motor::types::stub::config_t{});
    tfc::motor::api motor_api(i.conn, "", motor_conf);
    auto& stub = motor_api.stub();
    motor_api.run([&](auto ec) {
      i.res = ec;
      i.ran[0] = true;
    });
    stub.code = motor_error(frequency_drive_communication_fault);
    stub.tokens.notify_one();
    i.ctx.run_for(2ms);
    expect(i.ran[0]);
    expect(i.res == motor_error(frequency_drive_communication_fault));
  };
  "Stub test run speedratio"_test = [&] {
    instance i;
    std::shared_ptr<tfc::motor::api::config_t> motor_conf =
        std::make_shared<tfc::motor::api::config_t>(tfc::motor::types::stub::config_t{});
    tfc::motor::api motor_api(i.conn, "", motor_conf);
    auto& stub = motor_api.stub();
    motor_api.run(50 * speedratio_t::reference, [&](auto ec) {
      i.res = ec;
      i.ran[0] = true;
    });
    stub.code = motor_error(frequency_drive_communication_fault);
    stub.tokens.notify_one();
    i.ctx.run_for(2ms);
    expect(i.ran[0]);
    expect(i.res == motor_error(frequency_drive_communication_fault));
  };
  "Stub test run time"_test = [&] {
    instance i;
    std::shared_ptr<tfc::motor::api::config_t> motor_conf =
        std::make_shared<tfc::motor::api::config_t>(tfc::motor::types::stub::config_t{});
    tfc::motor::api motor_api(i.conn, "", motor_conf);
    auto& stub = motor_api.stub();
    motor_api.run(50 * s, [&](auto ec) {
      i.res = ec;
      i.ran[0] = true;
    });
    stub.code = motor_error(frequency_drive_communication_fault);
    stub.tokens.notify_one();
    i.ctx.run_for(2ms);
    expect(i.ran[0]);
    expect(i.res == motor_error(frequency_drive_communication_fault));
  };
  "Stub test convey speed and distance"_test = [&] {
    instance i;
    std::shared_ptr<tfc::motor::api::config_t> motor_conf =
        std::make_shared<tfc::motor::api::config_t>(tfc::motor::types::stub::config_t{});
    tfc::motor::api motor_api(i.conn, "", motor_conf);
    auto& stub = motor_api.stub();
    motor_api.convey(1 * mm / s, 1 * mm, [&](auto ec, auto travel) {
      i.res = ec;
      i.length = travel;
      i.ran[0] = true;
    });
    stub.code = motor_error(frequency_drive_communication_fault);
    stub.length = 10 * mm;
    stub.tokens.notify_one();
    i.ctx.run_for(2ms);
    expect(i.ran[0]);
    expect(i.res == motor_error(frequency_drive_communication_fault));
    expect(i.length == 10 * mm);
  };
  "Stub test convey speed"_test = [&] {
    instance i;
    std::shared_ptr<tfc::motor::api::config_t> motor_conf =
        std::make_shared<tfc::motor::api::config_t>(tfc::motor::types::stub::config_t{});
    tfc::motor::api motor_api(i.conn, "", motor_conf);
    auto& stub = motor_api.stub();
    motor_api.convey(1 * mm / s, [&](auto ec, auto travel) {
      i.res = ec;
      i.length = travel;
      i.ran[0] = true;
    });
    stub.code = motor_error(frequency_drive_communication_fault);
    stub.length = 10 * mm;
    stub.tokens.notify_one();
    i.ctx.run_for(2ms);
    expect(i.ran[0]);
    expect(i.res == motor_error(frequency_drive_communication_fault));
    expect(i.length == 10 * mm);
  };
  "Stub test convey distance"_test = [&] {
    instance i;
    std::shared_ptr<tfc::motor::api::config_t> motor_conf =
        std::make_shared<tfc::motor::api::config_t>(tfc::motor::types::stub::config_t{});
    tfc::motor::api motor_api(i.conn, "", motor_conf);
    auto& stub = motor_api.stub();
    stub.code = motor_error(frequency_drive_reports_fault);
    stub.length = 10 * mm;
    motor_api.convey(1 * micrometre_t::reference, [&](auto ec, auto travel) {
      i.res = ec;
      i.length = travel;
      i.ran[0] = true;
    });
    stub.tokens.notify_one();
    i.ctx.run_for(2ms);
    expect(i.ran[0]);
    expect(i.res == motor_error(frequency_drive_reports_fault));
    expect(i.length == 10 * mm) << i.length;
  };
  "Stub test convey canceled"_test = [&] {
    instance i;
    std::shared_ptr<tfc::motor::api::config_t> motor_conf =
        std::make_shared<tfc::motor::api::config_t>(tfc::motor::types::stub::config_t{});
    tfc::motor::api motor_api(i.conn, "", motor_conf);
    auto& stub = motor_api.stub();
    stub.code = {};
    stub.length = 10 * mm;
    motor_api.convey(1 * micrometre_t::reference, [&](auto ec, auto travel) {
      expect(ec == std::errc::operation_canceled) << ec.message();
      i.length = travel;
      i.ran[0] = true;
    });
    i.ctx.run_for(2ms);
    motor_api.run([&](auto) {});
    i.ctx.run_for(2ms);
    expect(i.ran[0]);
    expect(i.length == 10 * mm) << i.length;
  };
}
