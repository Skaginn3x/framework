#include <iostream>
#include <system_error>

#include <boost/asio/io_context.hpp>

#include <tfc/ipc/details/dbus_client_iface.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  boost::asio::io_context ctx;
  tfc::ipc_ruler::ipc_manager_client client(ctx);

  client.register_signal("signal", "", tfc::ipc::details::type_e::_bool, [](const std::error_code& ec) {
    if (ec) {
      std::cerr << "Error occurred" << ec.message() << '\n';
    }
  });
  client.register_slot("slot", "", tfc::ipc::details::type_e::_bool, [](const std::error_code& ec) {
    if (ec) {
      std::cerr << "Error occurred" << ec.message() << '\n';
    }
  });

  ctx.run_for(std::chrono::milliseconds(1));
  return 0;
}
