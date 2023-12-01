#include <memory>

#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>

#include <tfc/dbus/sd_bus.hpp>
#include <tfc/ipc.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;

auto main(int argc, char const* const* const argv) -> int {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};
  auto dbus{ std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system()) };
  dbus->request_name(tfc::dbus::make_dbus_process_name().c_str());

  bool in_1_value{ false };
  bool in_2_value{ false };

  tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client> out{ ctx, dbus, "out",
                                                                                          "Output of AND gate" };

  auto set_signal{ [&out, &in_1_value, &in_2_value]() {
    out.async_send(in_1_value && in_2_value, [](std::error_code err, std::size_t) {
      if (err) {
        fmt::print("Async send error: {}", err.message());
      }
    });
  } };

  tfc::ipc::slot<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client> in_1{
    ctx, dbus, "in_1", "Input 1 of AND gate",
    [&in_1_value, &set_signal]([[maybe_unused]] bool new_value) {
      in_1_value = new_value;
      set_signal();
    }
  };
  tfc::ipc::slot<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client> in_2{
    ctx, dbus, "in_2", "Input 2 of AND gate",
    [&in_2_value, &set_signal]([[maybe_unused]] bool new_value) {
      in_2_value = new_value;
      set_signal();
    }
  };

  ctx.run();

  return EXIT_SUCCESS;
}
