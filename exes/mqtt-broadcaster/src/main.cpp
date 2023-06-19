#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <tfc/confman/file_storage.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc/details/dbus_server_iface.hpp>
#include <tfc/progbase.hpp>

namespace as = boost::asio;
namespace am = async_mqtt;

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

auto send_message(std::string const& slot_name,
                  auto const& val,
                  [[maybe_unused]] as::io_context& ctx,
                  am::endpoint<am::role::client, am::protocol::mqtt>& amep) -> void {
  std::ostringstream oss;
  oss << val;
  std::string payload = "Slot: " + slot_name + ", Value: " + oss.str();

  std::cout << payload << "\n";

  boost::asio::strand<boost::asio::io_context::executor_type> strand(ctx.get_executor());
  std::allocator<void> allocator;

  strand.post([&amep, payload] () {
    amep.send(
        am::v3_1_1::publish_packet{ *amep.acquire_unique_packet_id(), am::allocate_buffer("topic1"),
                                    am::allocate_buffer(payload), am::qos::at_least_once },
        [](auto const& ec) { std::cout << "Publish: " << ec.message() << "\n"; }
    );
  }, allocator);



}

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  boost::asio::io_context ctx;

  // Initialize the MQTT client endpoint
  am::setup_log(am::severity_level::trace);
  as::ip::tcp::socket resolve_sock{ ctx };
  as::ip::tcp::resolver res{ resolve_sock.get_executor() };
  am::endpoint<am::role::client, am::protocol::mqtt> amep{ am::protocol_version::v3_1_1, ctx.get_executor() };

  // Resolve the MQTT broker hostname
  res.async_resolve(
      "localhost", "1883", [&]([[maybe_unused]] boost::system::error_code ec, as::ip::tcp::resolver::results_type eps) {
        // Underlying TCP connect
        as::async_connect(amep.next_layer(), eps,
                          [&]([[maybe_unused]] boost::system::error_code ec, as::ip::tcp::endpoint /*unused*/) {
                            // Send MQTT CONNECT
                            amep.send(
                                am::v3_1_1::connect_packet{
                                    true,         // clean_session
                                    0x1234,       // keep_alive
                                    am::allocate_buffer("cid1"),
                                    am::nullopt,  // will
                                    am::nullopt,  // username set like am::allocate_buffer("user1"),
                                    am::nullopt   // password set like am::allocate_buffer("pass1")
                                },
                                [&]([[maybe_unused]] am::system_error const& se) {
                                  amep.recv([&](am::packet_variant pv) {
                                    pv.visit(am::overload{ [&]([[maybe_unused]] am::v3_1_1::connack_packet const& p) {
                                                            // Do nothing here or do some initial setup if needed
                                                          },
                                                           [](auto const&) {} });
                                  });
                                });
                          });
      });

  // tfc::ipc_ruler::ipc_manager_client ipc_client(ctx);

  // std::vector<std::string> signals_on_client;

  // ipc_client.signals([&](const std::vector<tfc::ipc_ruler::signal>& signals) {
  //   for (auto const& signal : signals) {
  //     signals_on_client.push_back(signal.name);
  //   }
  // });

  // ctx.run_for(std::chrono::seconds(1));

  //// take out all strings that are banned
  // signals_on_client = take_out_bad_strings(signals_on_client);

  // for (auto const& signal : signals_on_client) {
  //   std::cout << "name: " << signal << std::endl;
  // }

  // std::vector<tfc::ipc::details::any_recv_cb> connect_slots;

  // timer for 3 seconds
  as::steady_timer timer(ctx);

  timer.async_wait([&](auto const&) { send_message("timer", "timer", ctx, amep); });

  timer.expires_from_now(std::chrono::seconds(3));

  // for (auto& signal_connect : signals_on_client) {
  //   connect_slots.emplace_back([&ctx, &amep](std::string_view sig) -> tfc::ipc::details::any_recv_cb {
  //     std::string slot_name = fmt::format("tfcctl_slot_{}", sig);

  //    std::cout << "slot name: " << slot_name << std::endl;

  //    auto ipc{ tfc::ipc::details::create_ipc_recv_cb<tfc::ipc::details::any_recv_cb>(ctx, slot_name) };
  //    std::visit(

  //        [&](auto&& receiver) {
  //          using receiver_t = std::remove_cvref_t<decltype(receiver)>;

  //          if constexpr (!std::same_as<std::monostate, receiver_t>) {
  //            auto error = receiver->init(

  //                sig, [&, sig, slot_name](auto const& val) { send_message(slot_name, val, ctx, amep); });

  //            if (error) {
  //              std::cout << "Failed to connect: " << error.message() << std::endl;
  //            }
  //          }
  //        },
  //        ipc);
  //    return ipc;
  //  }(signal_connect));
  //}

  ctx.run();
}
