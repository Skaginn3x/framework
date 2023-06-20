#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <tfc/ipc.hpp>
#include <boost/asio/yield.hpp>

#include <tfc/progbase.hpp>
#include <vector>

namespace as = boost::asio;
namespace am = async_mqtt;

struct app {
  app(as::ip::tcp::resolver& res,
      std::string host,
      std::string port,
      am::endpoint<am::role::client, am::protocol::mqtt>& amep,
      std::vector<std::string> signals_on_client,
      as::io_context& ctx)
      : res{ res }, host{ std::move(host) }, port{ std::move(port) }, amep{ amep },
        signals_on_client{ std::move(signals_on_client) }, ctx{ ctx } {}

  // forwarding callbacks
  void operator()() { proc({}, {}, {}, {}); }
  void operator()(boost::system::error_code const& ec) { proc(ec, {}, {}, {}); }
  void operator()(boost::system::error_code ec, as::ip::tcp::resolver::results_type eps) {
    proc(ec, {}, {}, std::move(eps));
  }
  void operator()(boost::system::error_code ec, as::ip::tcp::endpoint /*unused*/) { proc(ec, {}, {}, {}); }
  void operator()(am::system_error const& se) { proc({}, se, {}, {}); }
  void operator()(am::packet_variant pv) { proc({}, {}, am::force_move(pv), {}); }

private:
  void proc(boost::system::error_code const& ec,
            am::system_error const& se,
            am::packet_variant pv,
            std::optional<as::ip::tcp::resolver::results_type> eps) {
    reenter(coro) {

      // Resolve hostname
      res.async_resolve(host, port, *this);

      if (ec)
        return;

      // Underlying TCP connect
      co_await as::async_connect(amep.next_layer(), *eps, *this);

      if (ec)
        return;

      // Send MQTT CONNECT
      co_yield amep.send(
          am::v3_1_1::connect_packet{
              true,         // clean_session
              0x1234,       // keep_alive
              am::allocate_buffer("cid1"),
              am::nullopt,  // will
              am::nullopt,  // username set like am::allocate_buffer("user1"),
              am::nullopt   // password set like am::allocate_buffer("pass1")
          },
          *this);
      if (se) {
        return;
      }

      // Recv MQTT CONNACK
      co_yield amep.recv(*this);
      if (pv) {
        pv.visit(am::overload{ [&](am::v3_1_1::connack_packet const& p) {
                                std::cout << "MQTT CONNACK recv"
                                          << " sp:" << p.session_present() << std::endl;
                              },
                               [](auto const&) {} });
      } else {
        std::cout << "MQTT CONNACK recv error:" << pv.get<am::system_error>().what() << std::endl;
        return;
      }

      // Send MQTT SUBSCRIBE
      co_yield amep.send(am::v3_1_1::subscribe_packet{ *amep.acquire_unique_packet_id(),
                                                    { { am::allocate_buffer("signal_topic"), am::qos::at_most_once } } },
                      *this);

      if (se) {
        std::cout << "MQTT SUBSCRIBE send error:" << se.what() << std::endl;
        return;
      }

      // Recv MQTT SUBACK
      co_yield amep.recv(*this);

      if (pv) {
        pv.visit(am::overload{ [&](am::v3_1_1::suback_packet const& p) {
                                std::cout << "MQTT SUBACK recv"
                                          << " pid:" << p.packet_id() << " entries:";
                                for (auto const& e : p.entries()) {
                                  std::cout << e << " ";
                                }
                                std::cout << std::endl;
                              },
                               [](auto const&) {} });
      } else {
        std::cout << "MQTT SUBACK recv error:" << pv.get<am::system_error>().what() << std::endl;
        return;
      }

      std::cout << "Sending MQTT PUBLISH...\n";
      amep.send(am::v3_1_1::publish_packet{ *amep.acquire_unique_packet_id(), am::allocate_buffer("signal_topic"),
                                            am::allocate_buffer("signal_payload"), am::qos::at_least_once },
                *this);

      if (se) {
        std::cout << "MQTT PUBLISH send error:" << se.what() << std::endl;
        return;
      }

      std::vector<tfc::ipc::details::any_recv_cb> connect_slots;

      for (auto& signal_connect : signals_on_client) {

        connect_slots.emplace_back([&](std::string_view sig) -> tfc::ipc::details::any_recv_cb {
          std::string slot_name = fmt::format("tfcctl_slot_{}", sig);

          auto ipc{ tfc::ipc::details::create_ipc_recv_cb<tfc::ipc::details::any_recv_cb>(ctx, slot_name) };
          std::visit(

              [&](auto&& receiver) {
                using receiver_t = std::remove_cvref_t<decltype(receiver)>;

                if constexpr (!std::same_as<std::monostate, receiver_t>) {
                  auto error = receiver->init(

                      sig, [&, sig, slot_name](auto const& val) {
                        std::cout << "Received signal: " << sig << " value: " << val << std::endl;

                        std::stringstream ss;
                        ss << val;
                        std::string val_str = ss.str();

                        // Send MQTT PUBLISH
                        amep.send(am::v3_1_1::publish_packet{ *amep.acquire_unique_packet_id(), am::allocate_buffer(sig),
                                                              am::allocate_buffer(val_str), am::qos::at_least_once },
                                  *this);
                        if (se) {
                          std::cout << "MQTT PUBLISH send error:" << se.what() << std::endl;
                          return;
                        }
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
  }

  as::ip::tcp::resolver& res;
  std::string host;
  std::string port;
  am::endpoint<am::role::client, am::protocol::mqtt>& amep;
  std::vector<std::string> signals_on_client;
  as::io_context& ctx;
  std::size_t count = 0;
  as::coroutine coro;
};

std::vector<std::string> banned_strings = { "mainn", "ec_run", "operation-mode", "uint64_t" };

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

auto main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) -> int {
  as::io_context ioc;
  std::vector<std::string> signals_on_client = available_signals(ioc);
  as::ip::tcp::socket resolve_sock{ ioc };
  as::ip::tcp::resolver res{ resolve_sock.get_executor() };
  am::endpoint<am::role::client, am::protocol::mqtt> amep{ am::protocol_version::v3_1_1, ioc.get_executor() };
  app a{ res, "localhost", "1883", amep, signals_on_client, ioc };
  a();
  ioc.run();
}