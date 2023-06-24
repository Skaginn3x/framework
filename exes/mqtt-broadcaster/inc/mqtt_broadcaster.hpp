#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <iterator>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/unpack_properties.hpp>
#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/dbus/string_maker.hpp>
#include <tfc/ipc.hpp>
#include <tfc/logger.hpp>
#include <tfc/stx/to_tuple.hpp>

using std::cerr;
using std::endl;
using std::exception;
namespace po = boost::program_options;

using namespace std;
namespace asio = boost::asio;
using tfc::ipc::details::slot;

struct config {
  tfc::confman::observable<std::vector<std::string>> _banned_topics{};
  struct glaze {
    static constexpr auto value{ glz::object("_banned_topics", &config::_banned_topics) };
    static constexpr auto name{ "mqtt_broadcaster" };
  };
};

template <class IPCClient, class MatchClient, class MqttClient>
class mqtt_broadcaster {
public:
  mqtt_broadcaster(asio::io_context& ctx,
                   std::string const& broker_address,
                   std::string const& mqtt_port,
                   IPCClient& ipc_client)
      : _ctx(ctx), _ipc_client(ipc_client), _service_name(tfc::dbus::make_dbus_name("ipc_ruler")),
        _object_path(tfc::dbus::make_dbus_path("ipc_ruler")), _interface_name(tfc::dbus::make_dbus_name("manager")),
        _mqtt_client(std::make_shared<MqttClient>(async_mqtt::protocol_version::v3_1_1, ctx.get_executor())),
        _mqtt_host(broker_address), _mqtt_port(mqtt_port), _logger("mqtt_broadcaster"),
        _dbus_connection(std::make_unique<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system())),
        _signal_updates(
            std::make_unique<MatchClient>(*_dbus_connection,
                                          sdbusplus::bus::match::rules::propertiesChanged(_object_path, _interface_name),
                                          std::bind_front(&mqtt_broadcaster::update_signals, this))),
        _config(ctx, "mqtt_broadcaster"), _file_name("/etc/tfc/mqtt-broadcaster/def/mqtt_broadcaster.json"),
        _conf(ctx,
              _file_name,
              config{ ._banned_topics = tfc::confman::observable<std::vector<std::string>>{ std::vector<std::string>{} } }) {
    asio::co_spawn(_mqtt_client->strand(), mqtt_connect(), asio::detached);

    // TODO: this only works for the first file change, not the following file changes
    _conf->_banned_topics.observe([&, this](auto& new_conf, auto& old_conf) {
      for (auto const& topic : new_conf) {
        _logger.info("new topic: {}", topic);
      }

      for (auto const& topic : old_conf) {
        _logger.info("old topic: {}", topic);
      }

      _logger.info("new observable");
      _logger.info("new observable");
      _logger.info("new observable");
      _logger.info("new observable");
      _logger.info("new observable");
      ctx.run_for(std::chrono::milliseconds(1));
      restart();
    });

    ctx.run_for(std::chrono::milliseconds(1));

    // TODO: do we need this?, need to change for mock mqtt client in order to work
    asio::co_spawn(_mqtt_client->strand(), tfc::base::exit_signals(ctx), asio::detached);
    load_signals();
  }

private:
  auto load_signals() -> void {
    std::vector<tfc::ipc_ruler::signal> signals = get_signals();

    // connecting to all signals
    for (tfc::ipc_ruler::signal& signal : signals) {
      std::cout << "signal: " << signal.name << "\n";
      handle_signal(signal);
    }
  }

  // auto handle_signal(asio::io_context& ctx, auto& signal) -> void {
  auto handle_signal(tfc::ipc_ruler::signal& signal) -> void {
    switch (signal.type) {
      case tfc::ipc::details::type_e::_bool: {
        run_slot<tfc::ipc::details::type_bool, bool>(signal);
        break;
      }
      case tfc::ipc::details::type_e::_int64_t: {
        run_slot<tfc::ipc::details::type_int, int>(signal);
        break;
      }
      case tfc::ipc::details::type_e::_uint64_t: {
        run_slot<tfc::ipc::details::type_uint, uint>(signal);
        break;
      }
      case tfc::ipc::details::type_e::_double_t: {
        run_slot<tfc::ipc::details::type_double, double>(signal);
        break;
      }
      case tfc::ipc::details::type_e::_string: {
        run_slot<tfc::ipc::details::type_string, std::string>(signal);
        break;
      }
      case tfc::ipc::details::type_e::_json: {
        run_slot<tfc::ipc::details::type_json, std::string>(signal);
        break;
      }
      case tfc::ipc::details::type_e::unknown: {
        break;
      }
    }
  }

  // template the types away
  template <typename T, typename N>
  auto run_slot(tfc::ipc_ruler::signal& signal) -> void {
    _slots.push_back(std::make_shared<tfc::ipc::details::slot<T>>(_ctx, signal.name));
    auto& slot = std::get<std::shared_ptr<tfc::ipc::details::slot<T>>>(_slots.back());
    slot->connect(signal.name);
    asio::co_spawn(_mqtt_client->strand(), slot_coroutine<T, N>(*slot, signal.name), asio::detached);
  }

  // read all the signals again and restart the coroutine
  auto restart() -> void {
    _stop_coroutine = true;
    _logger.info("New signal has arrived, reloading signals");
    load_signals();
    _stop_coroutine = false;
  }

  //
  void update_signals([[maybe_unused]] sdbusplus::message::message& msg) noexcept { restart(); }

  // get all signals from the ipc manager
  auto get_signals() -> std::vector<tfc::ipc_ruler::signal> {
    std::vector<tfc::ipc_ruler::signal> signals_on_client;

    // store the found signals in a vector
    _ipc_client.signals([&](const std::vector<tfc::ipc_ruler::signal>& signals) {
      for (const tfc::ipc_ruler::signal& signal : signals) {
        signals_on_client.push_back(signal);
      }
    });

    // give the signals some time to arrive
    _ctx.run_for(std::chrono::milliseconds(20));

    // remove all signals that contain banned strings
    signals_on_client = clean_signals(signals_on_client);

    return signals_on_client;
  }

  // remove all signals that contain banned strings
  auto clean_signals(std::vector<tfc::ipc_ruler::signal> signals) -> std::vector<tfc::ipc_ruler::signal> {
    signals.erase(std::remove_if(signals.begin(), signals.end(),
                                 [&](const tfc::ipc_ruler::signal& signal) {
                                   for (std::string banned_string : _config->_banned_topics.value()) {
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
  auto slot_coroutine(tfc::ipc::details::slot<T>& slot, std::string signal_name) -> asio::awaitable<void> {
    _logger.log<tfc::logger::lvl_e::info>("starting coroutine for signal: {}", signal_name);
    std::replace(signal_name.begin(), signal_name.end(), '.', '/');

    while (!_stop_coroutine) {
      std::expected<N, std::error_code> msg = co_await slot.coro_receive();
      std::string value_string = convert_to_string(msg.value());
      co_await asio::post(_mqtt_client->strand(), asio::use_awaitable);

      if (msg) {
        _logger.log<tfc::logger::lvl_e::info>("sending message: {} to topic: {}", value_string, signal_name);
        auto result = co_await _mqtt_client->send(
            async_mqtt::v3_1_1::publish_packet{ _mqtt_client->acquire_unique_packet_id().value(),
                                                async_mqtt::allocate_buffer(signal_name),
                                                async_mqtt::allocate_buffer(value_string), async_mqtt::qos::at_least_once },
            asio::use_awaitable);

        if (result) {
          _logger.log<tfc::logger::lvl_e::error>("failed to connect to mqtt client: {}", result.message());
          co_await _mqtt_client->close(boost::asio::use_awaitable);

          asio::co_spawn(_ctx, mqtt_connect(), [&](std::exception_ptr ptr) {
            if (ptr) {
              std::rethrow_exception(ptr);
            }
          });
        }
      }
    }
  }

  auto mqtt_connect() -> asio::awaitable<void> {
    while (true) {
      try {
        asio::ip::tcp::socket resolve_sock{ _ctx };
        asio::ip::tcp::resolver res{ resolve_sock.get_executor() };
        asio::ip::tcp::resolver::results_type resolved_ip =
            co_await res.async_resolve(_mqtt_host, _mqtt_port, asio::use_awaitable);

        [[maybe_unused]] asio::ip::tcp::endpoint endpoint =
            co_await asio::async_connect(_mqtt_client->next_layer(), resolved_ip, asio::use_awaitable);

        co_await _mqtt_client->send(
            async_mqtt::v3_1_1::connect_packet{ true, 0x1234, async_mqtt::allocate_buffer("cid1"), async_mqtt::nullopt,
                                                async_mqtt::nullopt, async_mqtt::nullopt },
            asio::use_awaitable);

        async_mqtt::packet_variant packet_variant = co_await _mqtt_client->recv(asio::use_awaitable);  // connack

        co_await _mqtt_client->send(async_mqtt::v3_1_1::publish_packet{ _mqtt_client->acquire_unique_packet_id().value(),
                                                                        async_mqtt::allocate_buffer("test_topic"),
                                                                        async_mqtt::allocate_buffer("test_payload"),
                                                                        async_mqtt::qos::at_least_once },
                                    asio::use_awaitable);

        break;
      } catch (std::exception& e) {
        _logger.log<tfc::logger::lvl_e::error>("exception in mqtt_connect: {}", e.what());
        std::cout << "exception in mqtt_connect: " << e.what() << std::endl;
      }
      _logger.trace("retrying mqtt connection");
      co_await asio::steady_timer{ _ctx, std::chrono::milliseconds(20) }.async_wait(asio::use_awaitable);
    }
  }

  asio::io_context& _ctx;
  IPCClient _ipc_client;

  std::string _service_name;
  std::string _object_path;
  std::string _interface_name;

  std::shared_ptr<MqttClient> _mqtt_client;
  std::string _mqtt_host;
  std::string _mqtt_port;
  tfc::logger::logger _logger;

  std::unique_ptr<sdbusplus::asio::connection, std::function<void(sdbusplus::asio::connection*)>> _dbus_connection;
  std::unique_ptr<MatchClient, std::function<void(MatchClient*)>> _signal_updates;

  tfc::confman::config<config> _config;

  std::string const _file_name;

  tfc::confman::file_storage<config> _conf;

  std::vector<std::variant<std::shared_ptr<tfc::ipc::details::slot<tfc::ipc::details::type_bool>>,
                           std::shared_ptr<tfc::ipc::details::slot<tfc::ipc::details::type_string>>,
                           std::shared_ptr<tfc::ipc::details::slot<tfc::ipc::details::type_int>>,
                           std::shared_ptr<tfc::ipc::details::slot<tfc::ipc::details::type_uint>>,
                           std::shared_ptr<tfc::ipc::details::slot<tfc::ipc::details::type_double>>,
                           std::shared_ptr<tfc::ipc::details::slot<tfc::ipc::details::type_json>>>>
      _slots;

  std::vector<std::unique_ptr<sdbusplus::bus::match::match>> _signals;

  bool _stop_coroutine = false;
};