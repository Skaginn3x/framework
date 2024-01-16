#include <tfc/motor/atv320motor.hpp>

#include "atv320-server-side.hpp"

using tfc::motor::types::atv320motor;

struct clientinstance {
  clientinstance(instance& server) : slave_id{ server.slave_id }, client(server.dbus_connection, conf) {}

  std::uint16_t slave_id;
  atv320motor::config_t conf{ .slave_id = tfc::confman::observable<uint16_t>(slave_id) };
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
    clientinstance cinst(sinst);
    sinst.ctx.run_for(10ms);
    expect(cinst.client.connected());
  };
  "Call move home on client"_test = [] {
    instance inst;
    inst.ctx.run_for(5ms);
    clientinstance cinst(inst);
    inst.ctx.run_for(10ms);
    expect(cinst.client.connected());
    cinst.client.move_home([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::motor_home_sensor_unconfigured) << err.message();
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
    cinst.client.move(10 * mm, [&inst](const std::error_code& err, const decltype(10 * mm)& pos) {
      expect(tfc::motor::motor_enum(err) == err_enum::motor_home_sensor_unconfigured) << err.message();
      expect(pos == 0 * mm);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(10ms);
    expect(inst.ran[0]);
  };
  "Call speed and move on client"_test = [] {
    instance inst;
    inst.ctx.run_for(5ms);
    clientinstance cinst(inst);
    inst.ctx.run_for(10ms);
    expect(cinst.client.connected());
    cinst.client.move(
        10 * speedratio_t::reference, 10 * mm, [&inst](const std::error_code& err, const decltype(10 * mm)& pos) {
          expect(tfc::motor::motor_enum(err) == err_enum::motor_home_sensor_unconfigured) << err.message() << err.message();
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
    cinst.client.needs_homing([&inst](const std::error_code& err, bool needs_homing) {
      expect(tfc::motor::motor_enum(err) == err_enum::motor_home_sensor_unconfigured) << err.message();
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
    // Set an error on the drive to get an eary return from run. We are only testing dbus communication here. ctrl is tested
    // elsewhere.
    inst.ctrl.update_status(get_bad_status_missing_phase());
    cinst.client.run([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::frequency_drive_reports_fault) << err.message();
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
    // Set an error on the drive to get an eary return from run. We are only testing dbus communication here. ctrl is tested
    // elsewhere.
    inst.ctrl.update_status(get_bad_status_missing_phase());
    cinst.client.run(10 * s, [&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::frequency_drive_reports_fault) << err.message();
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
    // Set an error on the drive to get an eary return from run. We are only testing dbus communication here. ctrl is tested
    // elsewhere.
    inst.ctrl.update_status(get_bad_status_missing_phase());
    cinst.client.run(10 * speedratio_t::reference, 10 * s, [&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::frequency_drive_reports_fault) << err.message();
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
    // Set an error on the drive to get an eary return from run. We are only testing dbus communication here. ctrl is tested
    // elsewhere.
    inst.ctrl.update_status(get_bad_status_missing_phase());
    cinst.client.run(10 * speedratio_t::reference, [&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::frequency_drive_reports_fault) << err.message();
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(10ms);
    expect(inst.ran[0]);
  };
  "Run with direction"_test = [](auto direction) {
    instance inst;
    inst.ctx.run_for(5ms);
    clientinstance cinst(inst);
    inst.ctx.run_for(10ms);
    expect(cinst.client.connected());
    // Set an error on the drive to get an eary return from run. We are only testing dbus communication here. ctrl is tested
    // elsewhere.
    inst.ctrl.update_status(get_bad_status_missing_phase());
    cinst.client.run(direction, [&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::frequency_drive_reports_fault) << err.message();
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(10ms);
    expect(inst.ran[0]);
  } | std::vector{ tfc::motor::direction_e::backward, tfc::motor::direction_e::forward };
  "Run with direction and time"_test = [](auto direction) {
    instance inst;
    inst.ctx.run_for(5ms);
    clientinstance cinst(inst);
    inst.ctx.run_for(10ms);
    expect(cinst.client.connected());
    // Set an error on the drive to get an eary return from run. We are only testing dbus communication here. ctrl is tested
    // elsewhere.
    inst.ctrl.update_status(get_bad_status_missing_phase());
    cinst.client.run(direction, 21.1 * s, [&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::frequency_drive_reports_fault) << err.message();
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(10ms);
    expect(inst.ran[0]);
  } | std::vector{ tfc::motor::direction_e::backward, tfc::motor::direction_e::forward };
  "convey vel and travel"_test = [] {
    instance inst;
    inst.ctx.run_for(5ms);
    clientinstance cinst(inst);
    inst.ctx.run_for(10ms);
    expect(cinst.client.connected());
    cinst.client.convey(10 * mm / s, 10 * mm, [&inst](const std::error_code& err, const decltype(10 * mm)& pos) {
      expect(tfc::motor::motor_enum(err) == err_enum::motor_method_not_implemented) << err.message();
      expect(pos == 0 * mm);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(10ms);
    expect(inst.ran[0]);
  };
  "convey vel"_test = [] {
    instance inst;
    inst.ctx.run_for(5ms);
    clientinstance cinst(inst);
    inst.ctx.run_for(10ms);
    expect(cinst.client.connected());
    cinst.client.convey(10 * (mm / s), [&inst](const std::error_code& err, const micrometre_t& pos) {
      expect(tfc::motor::motor_enum(err) == err_enum::motor_method_not_implemented) << err.message();
      expect(pos == 0 * mm);
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
    cinst.client.convey(10 * (mm / s), 10 * s, [&inst](const std::error_code& err, const micrometre_t& pos) {
      expect(tfc::motor::motor_enum(err) == err_enum::motor_method_not_implemented) << err.message();
      expect(pos == 0 * mm);
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
    // Set an error on the drive to get an eary return from run. We are only testing dbus communication here. ctrl is tested
    // elsewhere.
    inst.ctrl.update_status(get_bad_status_missing_phase());
    cinst.client.convey(10 * mm, [&inst](const std::error_code& err, const decltype(10 * mm)& pos) {
      expect(tfc::motor::motor_enum(err) == err_enum::frequency_drive_reports_fault) << err.message();
      expect(pos == 0 * mm);
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
    cinst.client.stop([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::success) << err.message();
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
    cinst.client.quick_stop([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::success) << err.message();
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
    inst.ctrl.update_status(get_bad_status_missing_phase());
    cinst.client.reset([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::success) << err.message();
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    // Try to shorten the test time with an early return
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctx.run_for(10ms);
    expect(inst.ran[0]);
  };
}
