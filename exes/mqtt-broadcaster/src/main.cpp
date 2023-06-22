#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <tfc/ipc.hpp>

using namespace std;
namespace asio = boost::asio;
using tfc::ipc::details::slot;

template<tfc::ipc::details::type_e> struct type_mapping;

class mqtt_broadcaster {
public:
  mqtt_broadcaster(asio::io_context& ctx)
      : mqtt_client(std::make_shared<async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>>(
            async_mqtt::protocol_version::v3_1_1,
            ctx.get_executor())) {
    // connecting to mqtt client and throwing an exception if it fails
    asio::co_spawn(ctx, mqtt_connect(ctx, mqtt_client), [&](std::exception_ptr ptr) {
      if (ptr) {
        std::rethrow_exception(ptr);
      }
    });

    // getting all signals from the ipc manager
    std::vector<tfc::ipc_ruler::signal> signals = get_signals(ctx);

    asio::co_spawn(ctx, tfc::base::exit_signals(ctx), asio::detached);

    // connecting to all signals
    for (auto& signal : signals) {
      std::cout << "Connecting to signal: " << signal.name << "\n";

      switch (signal.type) {
        case tfc::ipc::details::type_e::_bool: {

          bool_slots.push_back(
              std::make_shared<tfc::ipc::details::slot<tfc::ipc::details::type_bool>>(ctx, signal.name));
          bool_slots.back()->connect(signal.name);
          asio::co_spawn(
              mqtt_client->strand(),
              slot_coroutine<tfc::ipc::details::type_bool, bool>(mqtt_client, *bool_slots.back(), signal.name),
              asio::detached);

          break;
        }
        case tfc::ipc::details::type_e::_int64_t: {
          std::cout << "slot value is uint64_t\n";

          int_slots.push_back(std::make_shared<tfc::ipc::details::slot<tfc::ipc::details::type_int>>(ctx, signal.name));
          int_slots.back()->connect(signal.name);
          asio::co_spawn(mqtt_client->strand(),
                         slot_coroutine<tfc::ipc::details::type_int, int>(mqtt_client, *int_slots.back(), signal.name),
                         asio::detached);

          break;
        }
        case tfc::ipc::details::type_e::_uint64_t: {
          std::cout << "slot value is uint64_t\n";

          uint_slots.push_back(
              std::make_shared<tfc::ipc::details::slot<tfc::ipc::details::type_uint>>(ctx, signal.name));
          uint_slots.back()->connect(signal.name);
          asio::co_spawn(
              mqtt_client->strand(),
              slot_coroutine<tfc::ipc::details::type_uint, uint>(mqtt_client, *uint_slots.back(), signal.name),
              asio::detached);

          break;
        }
        case tfc::ipc::details::type_e::_double_t: {
          std::cout << "slot value is double_t\n";

          double_slots.push_back(
              std::make_shared<tfc::ipc::details::slot<tfc::ipc::details::type_double>>(ctx, signal.name));
          double_slots.back()->connect(signal.name);
          asio::co_spawn(
              mqtt_client->strand(),
              slot_coroutine<tfc::ipc::details::type_double, double>(mqtt_client, *double_slots.back(), signal.name),
              asio::detached);

          break;
        }
        case tfc::ipc::details::type_e::_string: {
          std::cout << "slot value is string\n";

          string_slots.push_back(
              std::make_shared<tfc::ipc::details::slot<tfc::ipc::details::type_string>>(ctx, signal.name));
          string_slots.back()->connect(signal.name);
          asio::co_spawn(mqtt_client->strand(),
                         slot_coroutine<tfc::ipc::details::type_string, std::string>(mqtt_client, *string_slots.back(),
                                                                                     signal.name),
                         asio::detached);

          break;
        }
        case tfc::ipc::details::type_e::_json: {
          // slots.push_back(std::make_shared<Slot<tfc::ipc::details::type_json, std::string>>(ctx));
          std::cout << "slot value is json\n";
          break;
        }
        case tfc::ipc::details::type_e::unknown: {
          std::cout << "slot value is unknown\n";
          break;
        }
      }
    }
  }

private:

  // get all signals from the ipc manager
  auto get_signals(boost::asio::io_context& ctx) -> std::vector<tfc::ipc_ruler::signal> {
    tfc::ipc_ruler::ipc_manager_client ipc_client(ctx);
    std::vector<tfc::ipc_ruler::signal> signals_on_client;

    // store the found signals in a vector
    ipc_client.signals([&](const std::vector<tfc::ipc_ruler::signal>& signals) {
      for (auto const& signal : signals) {
        signals_on_client.push_back(signal);
      }
    });

    // TODO: might be unnecessary
    ctx.run_for(std::chrono::seconds(1));

    // remove all signals that contain banned strings
    signals_on_client = clean_signals(signals_on_client);

    return signals_on_client;
  }

  // remove all signals that contain banned strings
  auto clean_signals(std::vector<tfc::ipc_ruler::signal> signals) -> std::vector<tfc::ipc_ruler::signal> {
    signals.erase(std::remove_if(signals.begin(), signals.end(),
                                 [&](const tfc::ipc_ruler::signal& signal) {
                                   for (auto const& banned_string : banned_strings) {
                                     if (signal.name.find(banned_string) != std::string::npos) {
                                       return true;
                                     }
                                   }
                                   return false;
                                 }),
                  signals.end());

    return signals;
  }

  template <typename T>
  std::string convert_to_string(T value) {
    if constexpr (std::is_same_v<T, bool>) {
      return value ? "true" : "false";
    } else if constexpr (std::is_same_v<T, std::string>) {
      return value;
    } else if constexpr (std::is_arithmetic_v<T>) {
      return std::to_string(value);
    } else {
      return "unknown";
    }
  }

  template <class T, class N>
  auto slot_coroutine(auto amep, tfc::ipc::details::slot<T>& slot, std::string signal_name) -> asio::awaitable<void> {
    // string parsing for mqtt topic
    std::cout << "starting coroutine for : " << signal_name << "\n";
    std::replace(signal_name.begin(), signal_name.end(), '.', '/');

    while (true) {
      std::expected<N, std::error_code> msg = co_await slot.coro_receive();

      std::string value_string = convert_to_string(msg.value());

      co_await asio::post(amep->strand(), asio::use_awaitable);

      if (msg) {
        fmt::print("message={}\n", msg.value());
        co_await amep->send(
            async_mqtt::v3_1_1::publish_packet{ amep->acquire_unique_packet_id().value(),
                                                async_mqtt::allocate_buffer(signal_name),
                                                async_mqtt::allocate_buffer(value_string),
                                                async_mqtt::qos::at_least_once },
            asio::use_awaitable);

      } else {
        fmt::print("send error in sending ={}\n", msg.error().message());
      }
    }
  }

  auto mqtt_connect(asio::io_context& ctx, auto amep) -> asio::awaitable<void> {
    asio::ip::tcp::socket resolve_sock{ ctx };
    asio::ip::tcp::resolver res{ resolve_sock.get_executor() };
    asio::ip::tcp::resolver::results_type resolved_ip = co_await res.async_resolve("localhost", "1883", asio::use_awaitable);

    [[maybe_unused]] asio::ip::tcp::endpoint endpoint =
        co_await asio::async_connect(amep->next_layer(), resolved_ip, asio::use_awaitable);

    co_await amep->send(async_mqtt::v3_1_1::connect_packet{ true, 0x1234, async_mqtt::allocate_buffer("cid1"),
                                                            async_mqtt::nullopt, async_mqtt::nullopt, async_mqtt::nullopt },
                        asio::use_awaitable);

    [[maybe_unused]] async_mqtt::packet_variant packet_variant = co_await amep->recv(asio::use_awaitable);  // connack
    co_await amep->send(
        async_mqtt::v3_1_1::publish_packet{ amep->acquire_unique_packet_id().value(),
                                            async_mqtt::allocate_buffer("signal_topic"),
                                            async_mqtt::allocate_buffer("signal_payload"), async_mqtt::qos::at_least_once },
        asio::use_awaitable);
  }

  std::vector<std::string> banned_strings = {
    "mainn",   "ec_run", "operation-mode", "uint64_t", "tub_tipper", "ex_example_run_context", "ec_example_run_context",
    "ethercat"
  };

  std::shared_ptr<async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>> mqtt_client;

  std::vector<std::shared_ptr<tfc::ipc::details::slot<tfc::ipc::details::type_bool>>> bool_slots;
  std::vector<std::shared_ptr<tfc::ipc::details::slot<tfc::ipc::details::type_string>>> string_slots;
  std::vector<std::shared_ptr<tfc::ipc::details::slot<tfc::ipc::details::type_int>>> int_slots;
  std::vector<std::shared_ptr<tfc::ipc::details::slot<tfc::ipc::details::type_uint>>> uint_slots;
  std::vector<std::shared_ptr<tfc::ipc::details::slot<tfc::ipc::details::type_double>>> double_slots;
  std::vector<std::shared_ptr<tfc::ipc::details::slot<tfc::ipc::details::type_json>>> json_slots;

  std::vector<std::shared_ptr<tfc::ipc::details::slot<tfc::ipc::details::type_json>>> slots;

};

int main(int argc, char* argv[]) {
  tfc::base::init(argc, argv);
  asio::io_context ctx{};
  mqtt_broadcaster application(ctx);
  ctx.run();

  return 0;
}
