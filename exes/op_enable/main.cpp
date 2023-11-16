#include <cstddef>
#include <memory>

#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>

#include <system_error>
#include <tfc/ipc.hpp>
#include <tfc/operation_mode.hpp>
#include <tfc/operation_mode/common.hpp>

namespace asio = boost::asio;

auto main(int argc, char const* const* const argv) -> int {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};
  auto dbus{ std::make_shared<sdbusplus::asio::connection>(ctx) };
  tfc::logger::logger logger{ "actuator" };
  bool running{ false };

  tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client> sig{
    ctx, dbus, "out", "Pneumatic actuator or other output to activate only while operation running"
  };
  tfc::ipc::slot<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client> slot{
    ctx, dbus, "in", "Sensor or other input to propagate to output while operation running",
    [&running, &sig, &logger]([[maybe_unused]] bool new_value) {
      if (running) {
        sig.async_send(new_value, [&logger](std::error_code err, std::size_t) {
          if (err) {
            logger.error("Failed to send signal: {}", err.message());
          }
        });
      }
    }
  };

  tfc::operation::interface operations{ ctx, "operations" };
  operations.on_enter(tfc::operation::mode_e::running, [&running, &sig, &slot, &logger](tfc::operation::mode_e, tfc::operation::mode_e) {
    running = true;
    sig.async_send(slot.value().value_or(false), [&logger](std::error_code err, std::size_t) {
      if (err) {
        logger.error("Failed to send signal: {}", err.message());
      }
    });
  });
  operations.on_leave(tfc::operation::mode_e::running, [&running, &sig, &logger](tfc::operation::mode_e, tfc::operation::mode_e) {
    running = false;
    sig.async_send(false, [&logger](std::error_code err, std::size_t) {
      if (err) {
        logger.error("Failed to send signal: {}", err.message());
      }
    });
  });

  ctx.run();

  return EXIT_SUCCESS;
}
