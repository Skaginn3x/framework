#include <fmt/core.h>
#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <boost/asio/experimental/co_spawn.hpp>
#include <boost/asio/experimental/use_coro.hpp>
#include <tfc/ipc.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;
namespace am = async_mqtt;

auto slot_coro(tfc::ipc::details::slot<tfc::ipc::details::type_bool>& slot,
               std::shared_ptr<am::endpoint<am::role::client, am::protocol::mqtt>> amep, [[maybe_unused]] std::string slot_name) -> asio::awaitable<void> {
  while (true) {
    std::cout << "before coro wait\n";
    std::expected<bool, std::error_code> msg = co_await slot.coro_receive();
    std::cout << "after coro wait\n";

    // pipe boolean to stringstream
    std::stringstream ss;
    ss << std::boolalpha << msg.value();
    std::string s = ss.str();

    if (msg) {
      fmt::print("message={}\n", msg.value());
      co_await amep->send(
          am::v3_1_1::publish_packet{ *amep->acquire_unique_packet_id(), am::allocate_buffer("something"),
                                      am::allocate_buffer("something"), am::qos::exactly_once }, asio::use_awaitable);

    } else {
      fmt::print("error={}\n", msg.error().message());
    }
  }
}

std::vector<std::string> banned_strings = {
  "mainn",   "ec_run", "operation-mode", "uint64_t", "tub_tipper", "ex_example_run_context", "ec_example_run_context",
  "ethercat"
};

auto take_out_bad_strings(std::vector<std::string> signals) -> std::vector<std::string> {
  for (auto it = signals.begin(); it != signals.end();) {
    bool found = false;
    for (auto const& banned_string : banned_strings) {
      if (it->find(banned_string) != std::string::npos) {
        found = true;
        break;
      }
    }
    if (found) {
      it = signals.erase(it);
    } else {
      ++it;
    }
  }

  return signals;
}

auto available_signals([[maybe_unused]] boost::asio::io_context& ctx) -> std::vector<std::string> {
  tfc::ipc_ruler::ipc_manager_client ipc_client(ctx);
  std::vector<std::string> signals_on_client;

  ipc_client.signals([&](const std::vector<tfc::ipc_ruler::signal>& signals) {
    for (auto const& signal : signals) {
      signals_on_client.push_back(signal.name);
    }
  });

  ctx.run_for(std::chrono::seconds(1));
  signals_on_client = take_out_bad_strings(signals_on_client);

  return signals_on_client;
}

auto run(int argc,
         char* argv[],
         asio::io_context& ctx,
         std::shared_ptr<am::endpoint<am::role::client, am::protocol::mqtt>> amep) -> asio::awaitable<void> {
  tfc::base::init(argc, argv);

  asio::ip::tcp::socket resolve_sock{ ctx };
  asio::ip::tcp::resolver res{ resolve_sock.get_executor() };
  asio::ip::tcp::resolver::results_type resolved_ip = co_await res.async_resolve("localhost", "1883", asio::use_awaitable);

  [[maybe_unused]] asio::ip::tcp::endpoint endpoint =
      co_await asio::async_connect(amep->next_layer(), resolved_ip, asio::use_awaitable);

  co_await amep->send(
      am::v3_1_1::connect_packet{ true, 0x1234, am::allocate_buffer("cid1"), am::nullopt, am::nullopt, am::nullopt },
      asio::use_awaitable);

  [[maybe_unused]] am::packet_variant packet_variant = co_await amep->recv(asio::use_awaitable);

  co_await amep->send(am::v3_1_1::publish_packet{ *amep->acquire_unique_packet_id(), am::allocate_buffer("signal_topic"),
                                                  am::allocate_buffer("signal_payload"), am::qos::at_least_once },
                      asio::use_awaitable);

  tfc::ipc::details::slot<tfc::ipc::details::type_bool> slot{ ctx, "tfcctl_slot" };
  slot.connect("tfcctl.def.bool.bool.test");
  asio::co_spawn(ctx, slot_coro(slot, amep, "tfcctl_slot"), asio::detached);

  asio::co_spawn(ctx, tfc::base::exit_signals(ctx), asio::detached);

  ctx.run();
}

int main(int argc, char* argv[]) {
  asio::io_context ctx{};
  auto amep =
      std::make_shared<am::endpoint<am::role::client, am::protocol::mqtt>>(am::protocol_version::v3_1_1, ctx.get_executor());

  asio::co_spawn(ctx, run(argc, argv, ctx, amep), asio::detached);

  ctx.run();
  return 0;
}
