#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <variant>

#include <boost/asio.hpp>

#include <tfc/ipc.hpp>
#include <tfc/ipc/details/dbus_client_iface_mock.hpp>

#include <config/bridge_mock.hpp>
#include <spark_plug_interface.hpp>
#include <tfc_to_external.hpp>
#include "endpoint_mock.hpp"

namespace tfc::mqtt {

namespace asio = boost::asio;

class test_tfc_to_external {
public:
  explicit test_tfc_to_external() = default;

  auto test() -> asio::awaitable<bool> {
    using std::chrono::milliseconds;

    asio::io_context isolated_ctx{};

    ipc_ruler::ipc_manager_client_mock ipc_mock{ isolated_ctx };

    ipc::signal<ipc::details::type_bool, ipc_ruler::ipc_manager_client_mock&> sig(isolated_ctx, ipc_mock, "bool_signal");

    isolated_ctx.run_for(milliseconds{ 1 });

    sig.send(true);

    isolated_ctx.run_for(milliseconds{ 1 });

    config::bridge_mock config{ isolated_ctx, "test" };

    spark_plug_interface<config::bridge_mock,

                         client<endpoint_client_mock, config::bridge_mock> >
        sp_mock{ isolated_ctx, config };

    tfc_to_external<config::bridge_mock, client<endpoint_client_mock, config::bridge_mock>,
                    ipc_ruler::ipc_manager_client_mock&>
        tfc_ext_mock{ isolated_ctx, sp_mock, ipc_mock, config };

    isolated_ctx.run_for(milliseconds{ 1 });

    tfc_ext_mock.set_signals();

    isolated_ctx.run_for(milliseconds{ 1 });

    auto& first_signal = tfc_ext_mock.get_signals()[0];

    isolated_ctx.run_for(milliseconds{ 1 });

    if (!co_await std::visit(
            [](auto&& receiver) -> asio::awaitable<bool> {
              using receiver_t = std::remove_cvref_t<decltype(receiver)>;
              if constexpr (!std::same_as<receiver_t, std::monostate>) {
                co_return receiver->name() == "test_mqtt_bridge.def.bool.bool_signal";
              }
              co_return false;
            },
            first_signal)) {
      co_return false;
    }

    co_return co_await std::visit(
        [](auto&& receiver) -> asio::awaitable<bool> {
          using receiver_t = std::remove_cvref_t<decltype(receiver)>;
          if constexpr (std::same_as<receiver_t, ipc::details::bool_slot_cb_ptr>) {
            co_return receiver->value().value_or(false);
          }
          co_return false;
        },
        first_signal);
  }
};
}  // namespace tfc::mqtt
