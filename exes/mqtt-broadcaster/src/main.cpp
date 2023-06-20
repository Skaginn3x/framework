
#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <boost/asio/experimental/co_spawn.hpp>
#include <boost/asio/experimental/use_coro.hpp>
#include <tfc/ipc.hpp>

namespace asio = boost::asio;
namespace am = async_mqtt;

static asio::awaitable<void> make_coro(asio::io_context& ctx, std::vector<std::string> signals) {
  asio::ip::tcp::socket resolve_sock{ ctx };
  asio::ip::tcp::resolver res{ resolve_sock.get_executor() };
  am::endpoint<am::role::client, am::protocol::mqtt> amep{ am::protocol_version::v3_1_1, ctx.get_executor() };
  asio::ip::tcp::resolver::results_type resolved_ip = co_await res.async_resolve("localhost", "1883", asio::use_awaitable);

  [[maybe_unused]] asio::ip::tcp::endpoint endpoint =
      co_await asio::async_connect(amep.next_layer(), resolved_ip, asio::use_awaitable);

  co_await amep.send(
      am::v3_1_1::connect_packet{
          true,         // clean_session
          0x1234,       // keep_alive
          am::allocate_buffer("cid1"),
          am::nullopt,  // will
          am::nullopt,  // username set like am::allocate_buffer("user1"),
          am::nullopt   // password set like am::allocate_buffer("pass1")
      },
      asio::use_awaitable);

  [[maybe_unused]] am::packet_variant packet_variant = co_await amep.recv(asio::use_awaitable);
  // todo do something with the above packet

  co_await amep.send(am::v3_1_1::publish_packet{ *amep.acquire_unique_packet_id(), am::allocate_buffer("signal_topic"),
                                                 am::allocate_buffer("signal_payload"), am::qos::at_least_once },
                     asio::use_awaitable);

  for (auto const& signal : signals) {
    std::cout << "signal: " << signal << "\n";
  }

  std::vector<tfc::ipc::details::any_recv_cb> connect_slots;

  for (auto& signal_connect : signals) {
    connect_slots.emplace_back([&](std::string_view sig) -> tfc::ipc::details::any_recv_cb {
      std::string slot_name = fmt::format("tfcctl_slot_{}", sig);

      auto ipc{ tfc::ipc::details::create_ipc_recv_cb<tfc::ipc::details::any_recv_cb>(ctx, slot_name) };
      std::visit(

          [&](auto&& receiver) {
            using receiver_t = std::remove_cvref_t<decltype(receiver)>;

            if constexpr (!std::same_as<std::monostate, receiver_t>) {
              auto error =

                  receiver->init(

                  // change to awaitable function
                  sig,

                  [&, sig, slot_name](auto const& val) -> asio::awaitable<void> {
                    std::cout << "Received signal: " << sig << " value: " << val << std::endl;

                    std::stringstream ss;
                    ss << val;
                    std::string val_str = ss.str();

                    // Send MQTT PUBLISH
                    co_await amep.send(
                        am::v3_1_1::publish_packet{ *amep.acquire_unique_packet_id(), am::allocate_buffer("signal_topic"),
                                                    am::allocate_buffer("signal_payload"), am::qos::at_least_once },
                        asio::use_awaitable);
                  });

              if (error) {
                std::cout << "Failed to connect: " << error.message() << std::endl;
              }
            }
          },
          ipc);

      return ipc;
    }(signal_connect));
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

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
  asio::io_context ctx{};

  std::vector<std::string> signals_on_client = available_signals(ctx);

  asio::co_spawn(ctx, make_coro(ctx, signals_on_client), asio::detached);

  ctx.run();

  return EXIT_SUCCESS;
}
