#include <any>
#include <chrono>
#include <optional>

#include <boost/asio.hpp>

#include <spark_plug_interface.hpp>
#include <test_tfc_to_external.hpp>
#include <tfc/ipc.hpp>
#include <tfc_to_external.hpp>

namespace tfc::mqtt {

namespace asio = boost::asio;

auto test_tfc_to_external::run_io_context_for(asio::io_context& ctx, std::chrono::milliseconds duration) -> void {
  ctx.run_for(duration);
}

auto test_tfc_to_external::check_value(const std::optional<std::any>& value, bool expected_value) -> bool {
  if (!value.has_value()) {
    return false;
  }
  return std::any_cast<bool>(value.value()) == expected_value;
}

auto test_tfc_to_external::test() -> asio::awaitable<bool> {
  asio::io_context isolated_ctx{};
  spark_plug_mock sp_mock{ isolated_ctx };
  tfc_to_ext_mock tfc_ext_mock{ isolated_ctx, sp_mock };

  tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&> sig(
      isolated_ctx, tfc_ext_mock.ipc_client_, "bool_signal", "");

  run_io_context_for(isolated_ctx, std::chrono::milliseconds(20));

  tfc_ext_mock.set_signals();
  run_io_context_for(isolated_ctx, std::chrono::milliseconds(20));

  auto& first_signal = tfc_ext_mock.get_signals()[0];

  if (first_signal.information.name != "test_mqtt_bridge.def.bool.bool_signal") {
    co_return false;
  }

  run_io_context_for(isolated_ctx, std::chrono::milliseconds(20));

  if (!check_value(first_signal.current_value, false)) {
    co_return false;
  }

  co_await tfc_ext_mock.handle_msg(first_signal, std::optional<bool>(true));
  run_io_context_for(isolated_ctx, std::chrono::milliseconds(20));

  if (!check_value(first_signal.current_value, true)) {
    co_return false;
  }

  co_return true;
}

}  // namespace tfc::mqtt
