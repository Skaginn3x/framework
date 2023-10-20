#include <chrono>
#include <functional>
#include <vector>

#include <async_mqtt/buffer.hpp>
#include <async_mqtt/packet/v5_publish.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include <external_to_tfc.hpp>
#include <spark_plug_interface.hpp>
#include <tfc/logger.hpp>
#include <tfc_to_external.hpp>

namespace asio = boost::asio;

auto start(asio::io_context& io_ctx_) -> asio::awaitable<void> {
  tfc::logger::logger logger{ "run_loop" };
  while (true) {
    logger.trace("----------------------------------------------------------------------------");
    logger.trace("Event loop started");

    bool restart_needed = false;

    tfc::mqtt::spark_plug sp_interface{ io_ctx_ };

    tfc::mqtt::tfc_to_ext tfc_to_ext{ io_ctx_, sp_interface };

    tfc::mqtt::ext_to_tfc ext_to_tfc{ io_ctx_ };

    bool connection_success = co_await sp_interface.connect_mqtt_client();

    if (!connection_success) {
      continue;
    }

    ext_to_tfc.create_outward_signals();

    tfc_to_ext.set_signals();

    sp_interface.set_value_change_callback(std::bind_front(&tfc::mqtt::ext_to_tfc::receive_new_value, &ext_to_tfc));

    bool subscribe_success = co_await sp_interface.subscribe_to_ncmd();

    if (!subscribe_success) {
      continue;
    }

    co_await asio::steady_timer{ io_ctx_, std::chrono::seconds{ 1 } }.async_wait(asio::use_awaitable);

    auto signals = tfc_to_ext.get_signals();
    std::vector<asio::cancellation_signal> cancel_signals(signals.size() + 1);

    asio::co_spawn(sp_interface.strand(),
                   sp_interface.wait_for_payloads(std::bind_front(&tfc::mqtt::spark_plug::process_payload, &sp_interface),
                                                  restart_needed),
                   asio::bind_cancellation_slot(cancel_signals[0].slot(), asio::detached));

    for (std::size_t i = 0; i < signals.size(); ++i) {
      asio::co_spawn(sp_interface.strand(), tfc_to_ext.listen_to_signal(signals[i], restart_needed),
                     asio::bind_cancellation_slot(cancel_signals[i + 1].slot(), asio::detached));
    }

    while (!restart_needed) {
      co_await asio::steady_timer{ sp_interface.strand(), std::chrono::milliseconds{ 5 } }.async_wait(asio::use_awaitable);
    }

    for (std::size_t i = 0; i < signals.size(); ++i) {
      cancel_signals[i].emit(asio::cancellation_type::all);
    }
    logger.trace("Failure has occurred, restarting the program");
  }
}

auto main(int argc, char* argv[]) -> int {
  auto program_description{ tfc::base::default_description() };

  tfc::base::init(argc, argv, program_description);

  asio::io_context io_ctx{};

  asio::co_spawn(io_ctx, start(io_ctx), asio::detached);

  io_ctx.run();

  return 0;
}
