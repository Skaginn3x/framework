#pragma once

#include <chrono>
#include <functional>
#include <type_traits>

#include <boost/asio.hpp>

#include <tfc/logger.hpp>

#include <external_to_tfc.hpp>
#include <spark_plug_interface.hpp>
#include <tfc_to_external.hpp>

namespace asio = boost::asio;

namespace tfc::mqtt {
template <class config_t, class mqtt_client_t, class ipc_client_t>
class run {
  using spark_plug = spark_plug_interface<config_t, mqtt_client_t>;
  using ext_to_tfc = external_to_tfc<ipc_client_t&, config_t>;
  using tfc_to_ext = tfc_to_external<config_t, mqtt_client_t, ipc_client_t>;

public:
  explicit run(asio::io_context& io_ctx) : io_ctx_(io_ctx), ipc_client_(io_ctx) {}

  explicit run(asio::io_context& io_ctx, ipc_client_t ipc_client) : io_ctx_(io_ctx), ipc_client_(ipc_client) {
    static_assert(std::is_lvalue_reference<ipc_client_t>::value);
  }

  auto start() -> asio::awaitable<void> {
    while (true) {
      logger.trace("----------------------------------------------------------------------------");
      logger.trace("Event loop started");

      sp_interface_.emplace(io_ctx_, config_);

      if (!co_await sp_interface_->connect_mqtt_client()) {
        continue;
      }

      if (!co_await sp_interface_->subscribe_to_ncmd()) {
        continue;
      }

      exter_to_tfc_.emplace(io_ctx_, config_, ipc_client_);
      exter_to_tfc_->create_outward_signals();

      tfc_to_exter_.emplace(io_ctx_, sp_interface_.value(), ipc_client_, config_);
      tfc_to_exter_->set_signals();

      sp_interface_->set_value_change_callback(std::bind_front(&ext_to_tfc::receive_new_value, &exter_to_tfc_.value()));

      asio::cancellation_signal cancel_signal{};

      co_spawn(sp_interface_->strand(),
               sp_interface_->wait_for_payloads(std::bind_front(&spark_plug::process_payload, &sp_interface_.value())),
               bind_cancellation_slot(cancel_signal.slot(), asio::detached));

      co_await tfc_to_exter_->wait_for_restart(asio::use_awaitable);

      cancel_signal.emit(asio::cancellation_type::all);

      /// allow sockets to clear up and connections to close
      /// if this is skipped the next run will fail
      io_ctx_.run_for(std::chrono::milliseconds{ 10 });
    }
  }

  auto config() -> config_t& { return config_; }

private:
  asio::io_context& io_ctx_;
  ipc_client_t ipc_client_;
  config_t config_{ ipc_client_.connection(), "mqtt" };
  logger::logger logger{ "run_loop" };

  std::optional<spark_plug> sp_interface_;
  std::optional<tfc_to_ext> tfc_to_exter_;
  std::optional<ext_to_tfc> exter_to_tfc_;
};
}  // namespace tfc::mqtt
