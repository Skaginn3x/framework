#include <tfc/confman/detail/config_rpc_server.hpp>
#include <tfc/ipc_connector/dbus_server_iface.hpp>
#include <tfc/progbase.hpp>

#include <boost/ut.hpp>
#include <glaze/glaze.hpp>

namespace asio = boost::asio;

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  using boost::ut::operator""_test;
  using boost::ut::expect;
  using tfc::ipc::packet;
  using tfc::ipc::type_e;

  boost::asio::io_context ctx;
  tfc::ipc_ruler::ipc_manager_client client(ctx);

  client.register_signal("signal", tfc::ipc::type_e::_bool, [](const boost::system::error_code& ec) {
    if (ec) {
      std::cerr << "Error occured" << ec.what() << std::endl;
    }
  });
  client.register_slot(fmt::format("slot"), tfc::ipc::type_e::_bool, [](const boost::system::error_code& ec) {
    if (ec) {
      std::cerr << "Error occured" << ec.what() << std::endl;
    }
  });

  ctx.run_for(std::chrono::milliseconds(1));
  return 0;
}
