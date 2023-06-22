#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>
#include <exception>
#include <iostream>
#include <iterator>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/asio/property.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/unpack_properties.hpp>
#include <tfc/dbus/string_maker.hpp>
#include <tfc/ipc.hpp>
#include <tfc/logger.hpp>

using std::cerr;
using std::endl;
using std::exception;
namespace po = boost::program_options;

using namespace std;
namespace asio = boost::asio;
using tfc::ipc::details::slot;

class mqtt_broadcaster {
public:
  mqtt_broadcaster(asio::io_context& ctx, std::string const& broker_address, std::string const& mqtt_port)
      : _mqtt_client(std::make_shared<async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>>(
            async_mqtt::protocol_version::v3_1_1,
            ctx.get_executor())),
        _mqtt_host(broker_address), _mqtt_port(mqtt_port), _logger("mqtt_broadcaster") {
    asio::co_spawn(ctx, mqtt_connect(ctx, _mqtt_client), asio::detached);

    // getting all signals from the ipc manager
    testing_dbus(ctx);

    asio::co_spawn(ctx, tfc::base::exit_signals(ctx), asio::detached);

    // connecting to all signals
    // for (auto& signal : signals) {
    //   _logger.log<tfc::logger::lvl_e::info>("connecting to signal: {}", signal.name);

    //   switch (signal.type) {
    //     case tfc::ipc::details::type_e::_bool: {
    //       bool_slots.push_back(std::make_shared<tfc::ipc::details::slot<tfc::ipc::details::type_bool>>(ctx, signal.name));
    //       bool_slots.back()->connect(signal.name);
    //       asio::co_spawn(
    //           _mqtt_client->strand(),
    //           slot_coroutine<tfc::ipc::details::type_bool, bool>(ctx, _mqtt_client, *bool_slots.back(), signal.name),
    //           asio::detached);

    //       break;
    //     }
    //     case tfc::ipc::details::type_e::_int64_t: {
    //       int_slots.push_back(std::make_shared<tfc::ipc::details::slot<tfc::ipc::details::type_int>>(ctx, signal.name));
    //       int_slots.back()->connect(signal.name);
    //       asio::co_spawn(_mqtt_client->strand(),
    //                      slot_coroutine<tfc::ipc::details::type_int, int>(ctx, _mqtt_client, *int_slots.back(),
    //                      signal.name), asio::detached);
    //       break;
    //     }
    //     case tfc::ipc::details::type_e::_uint64_t: {
    //       uint_slots.push_back(std::make_shared<tfc::ipc::details::slot<tfc::ipc::details::type_uint>>(ctx, signal.name));
    //       uint_slots.back()->connect(signal.name);
    //       asio::co_spawn(
    //           _mqtt_client->strand(),
    //           slot_coroutine<tfc::ipc::details::type_uint, uint>(ctx, _mqtt_client, *uint_slots.back(), signal.name),
    //           asio::detached);
    //       break;
    //     }
    //     case tfc::ipc::details::type_e::_double_t: {
    //       double_slots.push_back(
    //           std::make_shared<tfc::ipc::details::slot<tfc::ipc::details::type_double>>(ctx, signal.name));
    //       double_slots.back()->connect(signal.name);
    //       asio::co_spawn(
    //           _mqtt_client->strand(),
    //           slot_coroutine<tfc::ipc::details::type_double, double>(ctx, _mqtt_client, *double_slots.back(),
    //           signal.name), asio::detached);
    //       break;
    //     }
    //     case tfc::ipc::details::type_e::_string: {
    //       string_slots.push_back(
    //           std::make_shared<tfc::ipc::details::slot<tfc::ipc::details::type_string>>(ctx, signal.name));
    //       string_slots.back()->connect(signal.name);
    //       asio::co_spawn(_mqtt_client->strand(),
    //                      slot_coroutine<tfc::ipc::details::type_string, std::string>(ctx, _mqtt_client,
    //                      *string_slots.back(),
    //                                                                                  signal.name),
    //                      asio::detached);
    //       break;
    //     }
    //     case tfc::ipc::details::type_e::_json: {
    //       break;
    //     }
    //     case tfc::ipc::details::type_e::unknown: {
    //       break;
    //     }
    //   }
    // }
  }

private:
  auto testing_dbus(asio::io_context& ctx) -> void {
    std::shared_ptr<sdbusplus::asio::connection> dbus_ =
        std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system());

    //// std::unique_ptr<sdbusplus::asio::object_server, std::function<void(sdbusplus::asio::object_server*)>>
    //// dbus_object_server_ = std::make_unique<sdbusplus::asio::object_server>(dbus_);

    //// std::shared_ptr<sdbusplus::asio::dbus_interface> dbus_interface_ =
    //// dbus_object_server_->add_interface("/com/skaginn3x/ipc_ruler", "com.skaginn3x.manager");

    //// dbus_interface_.get

    // auto match = std::make_unique<sdbusplus::bus::match::match>(
    //     *dbus_, "interface='com.skaginn3x.ipc_ruler'", [&](sdbusplus::message::message& message) {
    //       std::cout << "\n\nMessage received on signal\n"
    //                 << "Sender: " << message.get_sender()
    //                 << "\n"
    //                 //<< "Destination: " << message.get_destination() << "\n"
    //                 << "Interface: " << message.get_interface() << "\n"
    //                 << "Member: " << message.get_member() << "\n"
    //                 << "Path: " << message.get_path() << "\n";
    //     });

    // signals_.push_back(std::move(match));

    //// Run the event loop for a while to process signals

    std::string _service_name = tfc::dbus::make_dbus_name("ipc_ruler");
    std::string _object_path = tfc::dbus::make_dbus_path("ipc_ruler");
    std::string _interface_name = tfc::dbus::make_dbus_name("manager");

    std::cout << "service name: " << _service_name << "\n";
    std::cout << "object path: " << _object_path << "\n";
    std::cout << "interface name: " << _interface_name << "\n";

    std::unique_ptr<sdbusplus::asio::connection> connection_ =
        std::make_unique<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system());

    std::unique_ptr<sdbusplus::bus::match_t> match_ = std::make_unique<sdbusplus::bus::match_t>(
        *connection_,
        fmt::format("sender='{}',interface='{}',path='{}',type='signal'", _service_name, _interface_name, _object_path),
        [&](sdbusplus::message::message& message) {
          std::cout << "\n\nMessage received on signal\n"
                    << "Sender: " << message.get_sender()
                    << "\n"
                    //<< "Destination: " << message.get_destination() << "\n"
                    << "Interface: " << message.get_interface() << "\n"
                    << "Member: " << message.get_member() << "\n"
                    << "Path: " << message.get_path() << "\n";
        });

    ctx.run_for(std::chrono::seconds(1000));
  }

  // get all signals from the ipc manager
  auto get_signals(asio::io_context& ctx) -> std::vector<tfc::ipc_ruler::signal> {
    tfc::ipc_ruler::ipc_manager_client ipc_client(ctx);
    std::vector<tfc::ipc_ruler::signal> signals_on_client;

    // store the found signals in a vector
    ipc_client.signals([&](const std::vector<tfc::ipc_ruler::signal>& signals) {
      for (auto const& signal : signals) {
        signals_on_client.push_back(signal);
      }
    });

    // this is necessary for some reason, don't know why
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
  auto slot_coroutine(
      [[maybe_unused]] asio::io_context& ctx,
      std::shared_ptr<async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>>& coroutine_mqtt_client,
      tfc::ipc::details::slot<T>& slot,
      std::string signal_name) -> asio::awaitable<void> {
    // string parsing for mqtt topic
    _logger.log<tfc::logger::lvl_e::info>("starting coroutine for signal: {}", signal_name);
    std::replace(signal_name.begin(), signal_name.end(), '.', '/');

    while (true) {
      std::expected<N, std::error_code> msg = co_await slot.coro_receive();
      std::string value_string = convert_to_string(msg.value());
      co_await asio::post(coroutine_mqtt_client->strand(), asio::use_awaitable);

      if (msg) {
        _logger.log<tfc::logger::lvl_e::info>("sending message: {} to topic: {}", value_string, signal_name);
        auto result = co_await coroutine_mqtt_client->send(
            async_mqtt::v3_1_1::publish_packet{ coroutine_mqtt_client->acquire_unique_packet_id().value(),
                                                async_mqtt::allocate_buffer(signal_name),
                                                async_mqtt::allocate_buffer(value_string), async_mqtt::qos::at_least_once },
            asio::use_awaitable);

        if (result) {
          _logger.log<tfc::logger::lvl_e::error>("failed to connect to mqtt client: {}", result.message());
          co_await coroutine_mqtt_client->close(boost::asio::use_awaitable);

          asio::co_spawn(ctx, mqtt_connect(ctx, _mqtt_client), [&](std::exception_ptr ptr) {
            if (ptr) {
              std::rethrow_exception(ptr);
            }
          });
        }
      }
    }
  }

  auto mqtt_connect(asio::io_context& ctx, auto amep) -> asio::awaitable<void> {
    while (true) {
      try {
        asio::ip::tcp::socket resolve_sock{ ctx };
        asio::ip::tcp::resolver res{ resolve_sock.get_executor() };
        asio::ip::tcp::resolver::results_type resolved_ip =
            co_await res.async_resolve(_mqtt_host, _mqtt_port, asio::use_awaitable);

        [[maybe_unused]] asio::ip::tcp::endpoint endpoint =
            co_await asio::async_connect(amep->next_layer(), resolved_ip, asio::use_awaitable);

        co_await amep->send(
            async_mqtt::v3_1_1::connect_packet{ true, 0x1234, async_mqtt::allocate_buffer("cid1"), async_mqtt::nullopt,
                                                async_mqtt::nullopt, async_mqtt::nullopt },
            asio::use_awaitable);

        [[maybe_unused]] async_mqtt::packet_variant packet_variant = co_await amep->recv(asio::use_awaitable);  // connack
        co_await amep->send(async_mqtt::v3_1_1::publish_packet{ amep->acquire_unique_packet_id().value(),
                                                                async_mqtt::allocate_buffer("test_topic"),
                                                                async_mqtt::allocate_buffer("test_payload"),
                                                                async_mqtt::qos::at_least_once },
                            asio::use_awaitable);

        break;
      } catch (std::exception& e) {
        _logger.log<tfc::logger::lvl_e::error>("exception in mqtt_connect: {}", e.what());
      }
      co_await asio::steady_timer{ ctx, std::chrono::milliseconds(500) }.async_wait(asio::use_awaitable);
    }
  }

  std::vector<std::string> banned_strings = {
    "mainn",   "ec_run", "operation-mode", "uint64_t", "tub_tipper", "ex_example_run_context", "ec_example_run_context",
    "ethercat"
  };

  std::shared_ptr<async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>> _mqtt_client;
  std::string _mqtt_host;
  std::string _mqtt_port;
  tfc::logger::logger _logger;

  std::vector<std::shared_ptr<tfc::ipc::details::slot<tfc::ipc::details::type_bool>>> bool_slots;
  std::vector<std::shared_ptr<tfc::ipc::details::slot<tfc::ipc::details::type_string>>> string_slots;
  std::vector<std::shared_ptr<tfc::ipc::details::slot<tfc::ipc::details::type_int>>> int_slots;
  std::vector<std::shared_ptr<tfc::ipc::details::slot<tfc::ipc::details::type_uint>>> uint_slots;
  std::vector<std::shared_ptr<tfc::ipc::details::slot<tfc::ipc::details::type_double>>> double_slots;
  std::vector<std::shared_ptr<tfc::ipc::details::slot<tfc::ipc::details::type_json>>> json_slots;

  std::vector<std::shared_ptr<tfc::ipc::details::slot<tfc::ipc::details::type_json>>> slots;

  std::vector<std::unique_ptr<sdbusplus::bus::match::match>> signals_;
};

int main(int argc, char* argv[]) {
  auto prog_desc{ tfc::base::default_description() };
  std::string mqtt_host, mqtt_port;
  prog_desc.add_options()("mqtt_host", boost::program_options::value<std::string>(&mqtt_host)->required(),
                          "ip address of mqtt broker")(
      "mqtt_port", boost::program_options::value<std::string>(&mqtt_port)->required(), "port of mqtt broker");
  tfc::base::init(argc, argv, prog_desc);

  asio::io_context ctx{};
  mqtt_broadcaster application(ctx, mqtt_host, mqtt_port);
  ctx.run();

  return 0;
}
