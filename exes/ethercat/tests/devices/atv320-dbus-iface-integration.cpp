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
  clientinstance(instance& server, atv320motor::config_t conf, uint16_t slave_id = 999)
    : slave_id_{slave_id}, client(server.dbus_connection, conf){
  }

  std::uint16_t slave_id_;
  atv320motor::config_t conf{.slave_id = tfc::confman::observable<uint16_t>(slave_id_) };
  atv320motor client;
};

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
    // "Call move home on client"_test = [] {
    //   serverinstance sinst;
    //   sinst.ctx.run_for(5ms);
    //   clientinstance cinst(sinst);
    //   sinst.ctx.run_for(10ms);
    //   expect(cinst.client.connected());
    //   // Setup homing sensor on server instance
    //   expect(cinst.client.move_home());
    // };
}
