#pragma once

#include <boost/asio.hpp>

#include <tfc/logger.hpp>

#include <external_to_tfc.hpp>
#include <spark_plug_interface.hpp>
#include <tfc_to_external.hpp>

namespace asio = boost::asio;

namespace tfc::mqtt {

template <class config_t, class mqtt_client_t, class ipc_client_t>
class run {
public:
  explicit run(asio::io_context& io_ctx) : io_ctx_(io_ctx), ipc_client_(io_ctx) {}

  explicit run(asio::io_context& io_ctx, ipc_client_t ipc_client) : io_ctx_(io_ctx), ipc_client_(ipc_client) {
    static_assert(std::is_lvalue_reference<ipc_client_t>::value);
  }

  auto start() -> asio::awaitable<void> {
    // while (true) {
    logger.trace("----------------------------------------------------------------------------");
    logger.trace("Event loop started");

    //     using spark_plug = spark_plug_interface<config_t, mqtt_client_t>;
    //     spark_plug sp_interface{ io_ctx_, config_ };

    bool connection_success = co_await sp_interface_.connect_mqtt_client();

    if (!connection_success) {
      co_return;
      // continue;
    }

    bool subscribe_success = co_await sp_interface_.subscribe_to_ncmd();

    if (!subscribe_success) {
      // continue;
      co_return;
    }

    exter_to_tfc_.create_outward_signals();

    tfc_to_exter_.set_signals();

    sp_interface_.set_value_change_callback(std::bind_front(&ext_to_tfc::receive_new_value, &exter_to_tfc_));

    co_await asio::steady_timer{ io_ctx_, std::chrono::seconds{ 1 } }.async_wait(asio::use_awaitable);

    auto signals = tfc_to_exter_.get_signals();

    asio::cancellation_signal cancel_signal{};

    bool restart_needed = false;

    co_spawn(sp_interface_.strand(),
             // co_spawn(io_ctx_,
             sp_interface_.wait_for_payloads(std::bind_front(&spark_plug::process_payload, &sp_interface_), restart_needed),
             bind_cancellation_slot(cancel_signal.slot(), asio::detached));

    // io_ctx_.run_for(std::chrono::seconds{ 100 });

    while (!restart_needed) {
      co_await asio::steady_timer{ sp_interface_.strand(), std::chrono::seconds{ 5 } }.async_wait(asio::use_awaitable);
    }

    cancel_signal.emit(asio::cancellation_type::all);
    tfc_to_exter_.clear_signals();
    // }
    co_return;
  }

  asio::io_context& io_ctx_;
  ipc_client_t ipc_client_;
  config_t config_{ io_ctx_, "mqtt" };
  logger::logger logger{ "run_loop" };

  using spark_plug = spark_plug_interface<config_t, mqtt_client_t>;
  spark_plug sp_interface_{ io_ctx_, config_ };

  tfc_to_external<config_t, mqtt_client_t, ipc_client_t&> tfc_to_exter_{ io_ctx_, sp_interface_, ipc_client_, config_ };

  using any_signal_imc_mock = std::variant<std::monostate,                                        //
                                           ipc::signal<ipc::details::type_bool, ipc_client_t>,    //
                                           ipc::signal<ipc::details::type_int, ipc_client_t>,     //
                                           ipc::signal<ipc::details::type_uint, ipc_client_t>,    //
                                           ipc::signal<ipc::details::type_double, ipc_client_t>,  //
                                           ipc::signal<ipc::details::type_string, ipc_client_t>,  //
                                           ipc::signal<ipc::details::type_json, ipc_client_t>,
                                           ipc::signal<ipc::details::type_mass, ipc_client_t> >;

  using ext_to_tfc = external_to_tfc<ipc_client_t&, config_t, any_signal_imc_mock>;
  ext_to_tfc exter_to_tfc_{ io_ctx_, config_, ipc_client_ };
};
}  // namespace tfc::mqtt
