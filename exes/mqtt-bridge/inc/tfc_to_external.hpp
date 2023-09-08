#pragma once

#include <any>
#include <cmath>
#include <memory>
#include <optional>
#include <type_traits>
#include <vector>

#include <boost/asio.hpp>
#include <sdbusplus/bus/match.hpp>

#include <client.hpp>
#include <config/broker.hpp>
#include <endpoint.hpp>
#include <spark_plug_interface.hpp>
#include <structs.hpp>
#include <tfc/confman.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc/details/dbus_client_iface_mock.hpp>
#include <tfc/logger.hpp>

namespace tfc::mqtt {

namespace asio = boost::asio;

template <class spark_plug_config_t, class mqtt_client_t, class ipc_client_t>
class tfc_to_external {
public:
  explicit tfc_to_external(asio::io_context& io_ctx, spark_plug_interface<spark_plug_config_t, mqtt_client_t>& spark_plug_i);

  auto set_signals() -> void;

  auto handle_incoming_signals_from_ipc_client(std::vector<tfc::ipc_ruler::signal> const& signals) -> void;

  auto set_current_values() -> asio::awaitable<void>;

  auto async_receive_routine(auto&& receiver, bool& read_finished, structs::signal_data& signal) -> asio::awaitable<void>;

  auto set_initial_value(ipc::details::any_slot receiver, structs::signal_data& signal) -> asio::awaitable<void>;

  auto listen_to_signal(structs::signal_data& signal_data, bool& failure) -> asio::awaitable<void>;

  template <typename msg_t>
  auto handle_msg(structs::signal_data& signal_data, std::optional<msg_t> msg) -> asio::awaitable<void> {
    if (signal_data.current_value.has_value()) {
      // unsafe to compare floating point values with ==, so we use a tolerance
      if constexpr (std::is_floating_point_v<msg_t>) {
        if (std::abs(std::any_cast<msg_t>(signal_data.current_value.value()) - msg.value()) > 0.0001) {
          logger_.info("Value changed for signal: {}", signal_data.information.name);
          signal_data.current_value = msg.value();
          co_await spark_plug_interface_.update_value(signal_data);
        }
      } else {
        if (std::any_cast<msg_t>(signal_data.current_value.value()) != msg.value()) {
          logger_.info("Value changed for signal: {}", signal_data.information.name);
          signal_data.current_value = msg.value();
          co_await spark_plug_interface_.update_value(signal_data);
        }
      }
    } else {
      logger_.info("Value changed for signal: {}", signal_data.information.name);
      signal_data.current_value = msg.value();
      co_await spark_plug_interface_.update_value(signal_data);
    }
  }

  auto get_signals() -> std::vector<structs::signal_data>&;

private:
  asio::io_context& io_ctx_;
  spark_plug_interface<spark_plug_config_t, mqtt_client_t>& spark_plug_interface_;
  ipc_client_t ipc_client_{ io_ctx_ };
  tfc::logger::logger logger_{ "tfc_to_external" };
  std::unique_ptr<sdbusplus::bus::match::match> properties_callback_;
  std::vector<structs::signal_data> signals_;

  friend class test_tfc_to_external;
};

using tfc_to_ext = tfc_to_external<tfc::confman::config<config::spark_plug_b>,
                                   client<endpoint_client, tfc::confman::config<config::broker>>,
                                   tfc::ipc_ruler::ipc_manager_client>;

using tfc_to_ext_mock = tfc_to_external<config::spark_plug_b_mock,
                                        client<endpoint_client_mock, config::broker_mock>,
                                        tfc::ipc_ruler::ipc_manager_client_mock>;
}  // namespace tfc::mqtt
