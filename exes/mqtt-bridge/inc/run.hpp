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
    template<class config_t = confman::config<config::bridge>,
        class mqtt_client_t = client_n,
        class ipc_client_t = ipc_ruler::ipc_manager_client&>
    class run {
        using spark_plug = spark_plug_interface<config_t, mqtt_client_t>;
        using ext_to_tfc = external_to_tfc<ipc_client_t &, config_t>;
        using tfc_to_ext = tfc_to_external<config_t, mqtt_client_t, ipc_client_t>;

template <class config_t, class mqtt_client_t, class ipc_client_t>
class run {
public:
  explicit run(asio::io_context& io_ctx) : io_ctx_(io_ctx), ipc_client_(io_ctx) {}

        explicit run(asio::io_context &io_ctx, ipc_client_t ipc_client) : io_ctx_(io_ctx), ipc_client_(ipc_client) {
            static_assert(std::is_lvalue_reference<ipc_client_t>::value);
        }

        auto start() -> asio::awaitable<void> {
            while (true) {
                restart_needed_ = false;

                logger.trace("----------------------------------------------------------------------------");
                logger.trace("Event loop started");

                sp_interface_.emplace(io_ctx_, config_);
                tfc_to_exter_.emplace(io_ctx_, sp_interface_.value(), ipc_client_, config_, restart_needed_);
                exter_to_tfc_.emplace(io_ctx_, config_, ipc_client_);

                bool connection_success = co_await sp_interface_->connect_mqtt_client();

                if (!connection_success) {
                    continue;
                }

                bool subscribe_success = co_await sp_interface_->subscribe_to_ncmd();

                if (!subscribe_success) {
                    continue;
                }

                exter_to_tfc_->create_outward_signals();

                tfc_to_exter_->set_signals();

                sp_interface_->
                        set_value_change_callback(
                            std::bind_front(&ext_to_tfc::receive_new_value, &exter_to_tfc_.value()));

                asio::cancellation_signal cancel_signal{};

                co_spawn(
                    sp_interface_->strand(),
                    sp_interface_->wait_for_payloads(
                        std::bind_front(&spark_plug::process_payload, &sp_interface_.value()),
                        restart_needed_),
                    bind_cancellation_slot(cancel_signal.slot(), asio::detached));

                while (!restart_needed_) {
                    co_await asio::steady_timer{sp_interface_->strand(), std::chrono::seconds{1}}.async_wait(
                        asio::use_awaitable);
                }

                cancel_signal.emit(asio::cancellation_type::all);

                io_ctx_.run_for(std::chrono::seconds{1});
            }
        }

        auto config() -> config_t & { return config_; }

    private:
        asio::io_context &io_ctx_;
        ipc_client_t ipc_client_;
        bool restart_needed_{false};
        config_t config_{io_ctx_, "mqtt"};
        logger::logger logger{"run_loop"};

        std::optional<spark_plug> sp_interface_;
        std::optional<tfc_to_ext> tfc_to_exter_;
        std::optional<ext_to_tfc> exter_to_tfc_;
    };
} // namespace tfc::mqtt
