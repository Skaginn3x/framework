#include <boost/asio.hpp>
#include <tfc/confman/file_storage.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc/details/dbus_server_iface.hpp>
#include <tfc/progbase.hpp>
//#include <mqtt_client_cpp.hpp>

#include <iostream>
#include <string>

#include <boost/asio.hpp>

#include <async_mqtt/all.hpp>

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

// mqtt://mqtt.eclipse.org:1883
auto handle_message(std::string signal, auto const& val) -> void {
  std::cout << "signal: " << signal << " val: " << val << std::endl;

  am::endpoint<am::role::client, am::protocol::ws> amep {
    am::protocol_version::v3_1_1,
    ioc.get_executor()
  };

  as::ip::tcp::socket resolve_sock{ioc};
  as::ip::tcp::resolver res{resolve_sock.get_executor()};

  yield amep.send(
      am::v3_1_1::publish_packet{
          *amep.acquire_unique_packet_id(),
          am::allocate_buffer("topic1"),
          am::allocate_buffer("payload1"),
          am::qos::at_least_once
      },
      *this
  );

  auto c = MQTT_NS::make_async_client(ioc, argv[1], argv[2]);
  c->set_client_id("cid1");
  c->set_clean_session(true);
}

// this executable is supposed to connect to ipc-ruler, read a list of signals, make slots with callbacks for those signals
// and then broadcast them to mqtt
auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  boost::asio::io_context ctx;

  tfc::ipc_ruler::ipc_manager_client ipc_client(ctx);

  std::vector<std::string> signals_on_client;

  ipc_client.signals([&](const std::vector<tfc::ipc_ruler::signal>& signals) {
    for (auto const& signal : signals) {
      signals_on_client.push_back(signal.name);
    }
  });

  ctx.run_for(std::chrono::seconds(1));

  // take out all strings that are banned
  signals_on_client = take_out_bad_strings(signals_on_client);

  for (auto const& signal : signals_on_client) {
    std::cout << "name: " << signal << std::endl;
  }

  std::vector<tfc::ipc::details::any_recv_cb> connect_slots;

  for (auto& signal_connect : signals_on_client) {

    connect_slots.emplace_back([&ctx](std::string_view sig) -> tfc::ipc::details::any_recv_cb {

      std::string slot_name = fmt::format("tfcctl_slot_{}", sig);

      auto ipc{ tfc::ipc::details::create_ipc_recv_cb<tfc::ipc::details::any_recv_cb>(ctx, slot_name) };
      std::visit(

          [&](auto&& receiver) {

            using receiver_t = std::remove_cvref_t<decltype(receiver)>;

            if constexpr (!std::same_as<std::monostate, receiver_t>) {

              auto error = receiver->init(

                  sig, [&, sig, slot_name](auto const& val) {
                    handle_message(slot_name, val);
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

  ctx.run();

  return 0;
}
