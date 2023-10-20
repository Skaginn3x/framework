#include <chrono>
#include <concepts>
#include <string>
#include <system_error>
#include <utility>
#include <variant>
#include <vector>

#include <fmt/format.h>
#include <boost/asio.hpp>

#include <spark_plug_interface.hpp>
#include <structs.hpp>
#include <tfc/ipc.hpp>
#include <tfc_to_external.hpp>

namespace tfc::mqtt {

namespace asio = boost::asio;

template <class spark_plug_config_t, class mqtt_client_t, class ipc_client_t>
tfc_to_external<spark_plug_config_t, mqtt_client_t, ipc_client_t>::tfc_to_external(
    asio::io_context& io_ctx,
    tfc::mqtt::spark_plug_interface<spark_plug_config_t, mqtt_client_t>& spark_plug_i)
    : io_ctx_(io_ctx), spark_plug_interface_(spark_plug_i) {}

template <class spark_plug_config_t, class mqtt_client_t, class ipc_client_t>
auto tfc_to_external<spark_plug_config_t, mqtt_client_t, ipc_client_t>::set_signals() -> void {
  logger_.trace("Starting to add new signals...");
  ipc_client_.signals(
      [this](std::vector<ipc_ruler::signal> const& signals) { handle_incoming_signals_from_ipc_client(signals); });
}

template <class spark_plug_config_t, class mqtt_client_t, class ipc_client_t>
auto tfc_to_external<spark_plug_config_t, mqtt_client_t, ipc_client_t>::handle_incoming_signals_from_ipc_client(
    std::vector<ipc_ruler::signal> const& signals) -> void {
  logger_.trace("Received {} new signals to add.", signals.size());

  signals_.clear();
  signals_.reserve(signals.size());
  for (auto signal : signals) {
    const std::string slot_name{ fmt::format("{}_slot_mqtt_broadcaster_{}", ipc::details::enum_name(signal.type),
                                             signal.name) };

    auto slot = ipc::details::make_any_slot::make(signal.type, io_ctx_, slot_name);

    std::visit(
        [this, &signal]<typename recv_t>(recv_t&& receiver) {
          using receiver_t = std::remove_cvref_t<decltype(receiver)>;
          if constexpr (!std::same_as<receiver_t, std::monostate>) {
            auto error_code = receiver->connect(signal.name);
            if (error_code) {
              logger_.trace("Error connecting to signal: {}, error: {}", signal.name, error_code.message());
            }
          }
        },
        slot);

    signals_.emplace_back(signal, std::move(slot), std::nullopt);

    logger_.trace("Added signal_data for signal: {}", signal.name);
  }

  logger_.trace("All new signals added. Preparing to send NBIRTH and start signals...");

  asio::co_spawn(spark_plug_interface_.strand(), set_current_values(), asio::detached);
}

template <class spark_plug_config_t, class mqtt_client_t, class ipc_client_t>
auto tfc_to_external<spark_plug_config_t, mqtt_client_t, ipc_client_t>::get_signals() -> std::vector<structs::signal_data>& {
  return signals_;
}

template <class spark_plug_config_t, class mqtt_client_t, class ipc_client_t>
auto tfc_to_external<spark_plug_config_t, mqtt_client_t, ipc_client_t>::set_current_values() -> asio::awaitable<void> {
  logger_.info("Setting current values to interface");

  for (structs::signal_data& signal_data : signals_) {
    logger_.info("Processing signal_data: {}", signal_data.information.name);

    co_await std::visit(
        [&signal_data, this]<typename recv_t>(recv_t&& receiver) -> asio::awaitable<void> {
          using receiver_t = std::remove_cvref_t<decltype(receiver)>;
          if constexpr (!std::same_as<std::monostate, receiver_t>) {
            if (!signal_data.current_value.has_value()) {
              co_await set_initial_value(receiver, signal_data);
            }
          }
          co_return;
        },
        signal_data.receiver);

    logger_.info("Signal_data ({}) processed and added to the payload", signal_data.information.name);
  }

  spark_plug_interface_.set_current_values(signals_);
  spark_plug_interface_.send_current_values();
}

template <class spark_plug_config_t, class mqtt_client_t, class ipc_client_t>
auto tfc_to_external<spark_plug_config_t, mqtt_client_t, ipc_client_t>::async_receive_routine(auto&& receiver,
                                                                                              bool& read_finished,
                                                                                              structs::signal_data& signal)
    -> asio::awaitable<void> {
  auto val = co_await receiver->async_receive(asio::use_awaitable);
  if (val.has_value()) {
    signal.current_value = val.value();
  }
  read_finished = true;
}

// This function reads the initial value off the signal.
// The ideal solution is to use awaitable operators, but they are not compatible with azmq.
template <class spark_plug_config_t, class mqtt_client_t, class ipc_client_t>
auto tfc_to_external<spark_plug_config_t, mqtt_client_t, ipc_client_t>::set_initial_value(ipc::details::any_slot receiver,
                                                                                          structs::signal_data& signal)
    -> asio::awaitable<void> {
  bool read_finished = false;

  asio::steady_timer timer(co_await asio::this_coro::executor);
  timer.expires_after(std::chrono::milliseconds(10));

  asio::cancellation_signal cancel_signal;

  timer.async_wait([&read_finished, &cancel_signal](std::error_code error_code) {
    if (!error_code) {
      cancel_signal.emit(asio::cancellation_type::all);
      read_finished = true;
    }
  });

  std::visit(
      [this, &read_finished, &signal, &cancel_signal]<typename recv_t>(recv_t&& recv) {
        using receiver_t = std::remove_cvref_t<decltype(recv)>;
        if constexpr (!std::same_as<std::monostate, receiver_t>) {
          asio::co_spawn(spark_plug_interface_.strand(),
                         async_receive_routine(std::forward<decltype(recv)>(recv), read_finished, signal),
                         asio::bind_cancellation_slot(cancel_signal.slot(), asio::detached));
        }
      },
      receiver);

  while (!read_finished) {
    co_await asio::steady_timer{ spark_plug_interface_.strand(), std::chrono::milliseconds{ 1 } }.async_wait(
        asio::use_awaitable);
  }
  timer.cancel();
  co_return;
}

template <class spark_plug_config_t, class mqtt_client_t, class ipc_client_t>
auto tfc_to_external<spark_plug_config_t, mqtt_client_t, ipc_client_t>::listen_to_signal(structs::signal_data& signal_data,
                                                                                         bool& failure_occurred)
    -> asio::awaitable<void> {
  co_await std::visit(
      [&]<typename recv_t>(recv_t& receiver) -> asio::awaitable<void> {
        using r_t = std::remove_cvref_t<decltype(receiver)>;
        if constexpr (!std::same_as<std::monostate, r_t>) {
          while (true) {
            logger_.trace("Waiting for message for signal: {}", signal_data.information.name);

            // this line has segfault
            auto msg = co_await receiver->async_receive(asio::use_awaitable);
            logger_.trace("Received a new message for signal: {}", signal_data.information.name);

            if (msg) {
              co_await handle_msg<std::remove_cvref_t<decltype(msg.value())>>(signal_data, msg.value());
            } else {
              logger_.error("Failed to receive message for signal: {}", signal_data.information.name);
              failure_occurred = true;
            }
          }
        } else {
          logger_.warn("Receiver is not initialized for signal: {}", signal_data.information.name);
          failure_occurred = true;
        }
      },
      signal_data.receiver);
  co_return;
}

}  // namespace tfc::mqtt

template class tfc::mqtt::tfc_to_external<tfc::mqtt::config::spark_plug_b_mock,
                                          tfc::mqtt::client<tfc::mqtt::endpoint_client_mock, tfc::mqtt::config::broker_mock>,
                                          tfc::ipc_ruler::ipc_manager_client_mock>;

template class tfc::mqtt::tfc_to_external<
    tfc::confman::config<tfc::mqtt::config::spark_plug_b>,
    tfc::mqtt::client<tfc::mqtt::endpoint_client, tfc::confman::config<tfc::mqtt::config::broker>>,
    tfc::ipc_ruler::ipc_manager_client>;
