#include <tfc/motor/atv320motor.hpp>

#include "atv320-server-side.hpp"


using tfc::motor::types::atv320motor;
// struct serverinstance {
//   serverinstance() {
//   }
//
//   asio::io_context ctx{ asio::io_context() };
//   std::shared_ptr<sdbusplus::asio::connection> dbus_connection{ std::make_shared<sdbusplus::asio::connection>(ctx) };
//   std::uint16_t slave_id{ 999 };
//   tfc::ipc_ruler::ipc_manager_client_mock manager{ dbus_connection };
//   using timer_t = asio::basic_waitable_timer<tfc::testing::clock, tfc::testing::wait_traits>;
//   controller<tfc::ipc_ruler::ipc_manager_client_mock, tfc::confman::stub_config, timer_t, bool_slot_t> ctrl{ dbus_connection,
//                                                                                                              manager,
//                                                                                                              slave_id };
//   atv320motor::config_t atv_conf;
//   std::array<bool, 10> ran{};
// };

struct clientinstance {
  clientinstance(instance& server, atv320motor::config_t conf = {}, uint16_t slave_id = 999)
    : slave_id_{ slave_id }, client(server.dbus_connection, conf) {
  }

  std::uint16_t slave_id_;
  atv320motor::config_t conf{ .slave_id = tfc::confman::observable<uint16_t>(slave_id_) };
  atv320motor client;
};

using mp_units::si::unit_symbols::mm;
using mp_units::si::unit_symbols::s;

auto main(int, char const* const* argv) -> int {
  std::array<char const*, 4> args{ argv[0], "--log-level", "trace", "--stdout" };
  tfc::base::init(args.size(), args.data());
  "test ping from client to server"_test = [] {
    instance sinst;
    sinst.ctx.run_for(5ms);
    clientinstance cinst(sinst, {}, 999);
    sinst.ctx.run_for(10ms);
    expect(cinst.client.connected());
  };
  "Call move home on client"_test = [] {
    instance inst;
    inst.ctx.run_for(5ms);
    clientinstance cinst(inst);
    inst.ctx.run_for(10ms);
    expect(cinst.client.connected());
    // Setup homing sensor on server instance
    cinst.client.move_home([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::motor_home_sensor_unconfigured);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(10ms);
    expect(inst.ran[0]);
  };
  "Call move on client"_test = [] {
    instance inst;
    inst.ctx.run_for(5ms);
    clientinstance cinst(inst);
    inst.ctx.run_for(10ms);
    expect(cinst.client.connected());
    // Setup homing sensor on server instance
    cinst.client.move(10 * mm, [&inst](const std::error_code& err, const decltype(10 * mm)& pos) {
      expect(tfc::motor::motor_enum(err) == err_enum::motor_home_sensor_unconfigured);
      expect(pos == 0 * mm);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(10ms);
    expect(inst.ran[0]);
  };
  "Call move on client"_test = [] {
    instance inst;
    inst.ctx.run_for(5ms);
    clientinstance cinst(inst);
    inst.ctx.run_for(10ms);
    expect(cinst.client.connected());
    // Setup homing sensor on server instance
    cinst.client.move(10 * speedratio_t::reference, 10 * mm,
                      [&inst](const std::error_code& err, const decltype(10 * mm)& pos) {
                        expect(tfc::motor::motor_enum(err) == err_enum::motor_home_sensor_unconfigured);
                        expect(pos == 0 * mm);
                        inst.ran[0] = true;
                        inst.ctx.stop();
                      });
    inst.ctx.run_for(10ms);
    expect(inst.ran[0]);
  };
  "needs homing"_test = [] {
    instance inst;
    inst.ctx.run_for(5ms);
    clientinstance cinst(inst);
    inst.ctx.run_for(10ms);
    expect(cinst.client.connected());
    // Setup homing sensor on server instance
    cinst.client.needs_homing([&inst](const std::error_code& err, bool needs_homing) {
      expect(tfc::motor::motor_enum(err) == err_enum::motor_home_sensor_unconfigured);
      expect(needs_homing);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(10ms);
    expect(inst.ran[0]);
  };
  "Run"_test = [] {
    instance inst;
    inst.ctx.run_for(5ms);
    clientinstance cinst(inst);
    inst.ctx.run_for(10ms);
    expect(cinst.client.connected());
    // Setup homing sensor on server instance
    cinst.client.run([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::motor_general_error);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(10ms);
    expect(inst.ran[0]);
  };
  "Run with time"_test = [] {
    instance inst;
    inst.ctx.run_for(5ms);
    clientinstance cinst(inst);
    inst.ctx.run_for(10ms);
    expect(cinst.client.connected());
    // Setup homing sensor on server instance
    cinst.client.run(10 * s, [&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::motor_general_error);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(10ms);
    expect(inst.ran[0]);
  };
  "Run with time and speed"_test = [] {
    instance inst;
    inst.ctx.run_for(5ms);
    clientinstance cinst(inst);
    inst.ctx.run_for(10ms);
    expect(cinst.client.connected());
    // Setup homing sensor on server instance
    cinst.client.run(10 * speedratio_t::reference, 10 * s, [&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::motor_general_error);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(10ms);
    expect(inst.ran[0]);
  };
  "Run with speed"_test = [] {
    instance inst;
    inst.ctx.run_for(5ms);
    clientinstance cinst(inst);
    inst.ctx.run_for(10ms);
    expect(cinst.client.connected());
    // Setup homing sensor on server instance
    cinst.client.run(10 * speedratio_t::reference, [&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::motor_general_error);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(10ms);
    expect(inst.ran[0]);
  };
  "convey vel and travel"_test = [] {
    instance inst;
    inst.ctx.run_for(5ms);
    clientinstance cinst(inst);
    inst.ctx.run_for(10ms);
    expect(cinst.client.connected());
    // Setup homing sensor on server instance
    cinst.client.convey(10 * mm / s, 10 * mm, [&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::motor_general_error);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(10ms);
    expect(inst.ran[0]);
  };
  "convey vel and time"_test = [] {
    instance inst;
    inst.ctx.run_for(5ms);
    clientinstance cinst(inst);
    inst.ctx.run_for(10ms);
    expect(cinst.client.connected());
    // Setup homing sensor on server instance
    cinst.client.convey(10 * mm / s, 10 * s, [&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::motor_general_error);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(10ms);
    expect(inst.ran[0]);
  };
  "convey travel"_test = [] {
    instance inst;
    inst.ctx.run_for(5ms);
    clientinstance cinst(inst);
    inst.ctx.run_for(10ms);
    expect(cinst.client.connected());
    // Setup homing sensor on server instance
    cinst.client.convey(10 * mm, [&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::motor_general_error);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(10ms);
    expect(inst.ran[0]);
  };
  "stop"_test = [] {
    instance inst;
    inst.ctx.run_for(5ms);
    clientinstance cinst(inst);
    inst.ctx.run_for(10ms);
    expect(cinst.client.connected());
    // Setup homing sensor on server instance
    cinst.client.stop([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::motor_general_error);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(10ms);
    expect(inst.ran[0]);
  };
  "quick_stop"_test = [] {
    instance inst;
    inst.ctx.run_for(5ms);
    clientinstance cinst(inst);
    inst.ctx.run_for(10ms);
    expect(cinst.client.connected());
    // Setup homing sensor on server instance
    cinst.client.quick_stop([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::motor_general_error);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(10ms);
    expect(inst.ran[0]);
  };
  "reset"_test = [] {
    instance inst;
    inst.ctx.run_for(5ms);
    clientinstance cinst(inst);
    inst.ctx.run_for(10ms);
    expect(cinst.client.connected());
    // Setup homing sensor on server instance
    cinst.client.reset([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::frequency_drive_reports_fault);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(10ms);
    expect(inst.ran[0]);
  };
}
