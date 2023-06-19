#include <async_mqtt/all.hpp>
#include <boost/ut.hpp>
#include <sdbusplus/test/sdbus_mock.hpp>
#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/ipc.hpp>
#include <tfc/stubs/confman.hpp>
#include "config.hpp"
#include "mqtt_broadcaster.hpp"

namespace ut = boost::ut;
using boost::ut::operator""_test;

constexpr std::chrono::duration timeout_duration = std::chrono::milliseconds(25);

// This class mocks the response to send for the MQTT client
class result {
public:
  explicit result(std::string& message) : message_(message) {}

  explicit operator bool() const { return message_ != "Success"; }

  [[nodiscard]] auto message() -> std::string { return message_; }

private:
  std::string message_;
};

// This class mocks the responses that would arrive from the MQTT client
class mock_mqtt_client {
public:
  mock_mqtt_client(async_mqtt::protocol_version, const boost::asio::io_context::executor_type& executor)
      : strand_(executor), socket_(executor.context()), ctx_(executor) {}

  // This function returns the strand (consecutive execution context) of the mock client
  auto strand() -> boost::asio::strand<boost::asio::io_context::executor_type>& { return strand_; }

  // This function sends a publishing packet (normal packet) to the mock client
  auto send(async_mqtt::v5::publish_packet packet, const boost::asio::use_awaitable_t<>&) -> boost::asio::awaitable<result> {
    std::cout << "publish packet arrived: " << packet.payload()[0] << "\n";
    std::cout << "publish packet arrived: " << packet.topic() << "\n";
    if (online_) {
      messages_.push_back(packet);
      std::string message = "Success";
      co_return result{ message };
    } else {
      std::string message = "Failure";
      co_return result{ message };
    }
  }

  // This function sends a connect packet to the mock client
  auto send([[maybe_unused]] async_mqtt::v5::connect_packet packet, const boost::asio::use_awaitable_t<>&)
      -> boost::asio::awaitable<result> {
    if (online_) {
      std::string message = "Success";
      co_return result{ message };
    } else {
      std::string message = "Failure";
      co_return result{ message };
    }
  }

  // This function returns the last packet that was sent to the mock client
  auto get_last_message() -> async_mqtt::v5::publish_packet {
    if (messages_.empty()) {
      throw std::exception();
    }
    return messages_.back();
  }

  // This function returns a unique packet ID that is consecutively incremented
  auto acquire_unique_packet_id() -> std::optional<uint16_t> {
    packet_id_++;
    return packet_id_;
  }

  // This function should be called to close the socket of the MQTT client but for mocking purposes it is not strictly needed
  static auto close(boost::asio::use_awaitable_t<>) -> boost::asio::awaitable<void> { co_return; }

  // This function is called when a program wants to wait for the next packet that is received on the MQTT client
  auto recv(boost::asio::use_awaitable_t<>) -> boost::asio::awaitable<async_mqtt::packet_variant> {
    if (online_) {
      co_return async_mqtt::v5::connack_packet{ true, async_mqtt::connect_reason_code::success };
    } else {
      co_return async_mqtt::v5::connack_packet{ false, async_mqtt::connect_reason_code::server_unavailable };
    }
  }

  // This function is the same as the previous one, but it allows the user to specify which packets to receive.
  auto recv(async_mqtt::filter, std::set<async_mqtt::control_packet_type>, boost::asio::use_awaitable_t<>)
      -> boost::asio::awaitable<async_mqtt::packet_variant> {
    if (online_) {
      co_return async_mqtt::v5::connack_packet{ false, async_mqtt::connect_reason_code::success };
    } else {
      co_return async_mqtt::v5::connack_packet{ false, async_mqtt::connect_reason_code::server_unavailable };
    }
  }

  // This function returns the socket of the MQTT client
  auto next_layer() -> boost::asio::ip::tcp::socket& { return socket_; }

  // This function allows the testing program to mock that the MQTT client is offline
  auto set_online(bool online) -> void { this->online_ = online; }

private:
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
  boost::asio::ip::tcp::socket socket_;
  boost::asio::io_context::executor_type ctx_;
  bool online_ = true;
  std::vector<async_mqtt::v5::publish_packet> messages_;
  uint16_t packet_id_ = 0;
};

template <typename storage_t>
class file_testable : public tfc::confman::file_storage<storage_t> {
public:
  using tfc::confman::file_storage<storage_t>::file_storage;

  ~file_testable() {
    std::error_code ignore{};
    std::filesystem::remove(this->file(), ignore);
  }
};

// This function tests the sending of a simple value (bool) to the MQTT client
static auto send_simple_value(
    asio::io_context& ctx,
    std::shared_ptr<mock_mqtt_client> mqtt_client,
    tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock>& signal) -> void {
  ctx.run_for(timeout_duration);
  auto last_message = mqtt_client->get_last_message();

  ut::expect(last_message.packet_id() == 1) << last_message.packet_id();
  ut::expect(last_message.topic() == "mqtt-broadcaster-tests/def/bool/test_signal")
      << "topic should be: " << last_message.topic();
  ut::expect(last_message.payload()[0] == "false") << "payload should be: " << last_message.payload()[0];
  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos should be: " << last_message.opts().get_qos();

  signal.send(true);
  ctx.run_for(timeout_duration);
  last_message = mqtt_client->get_last_message();

  ut::expect(last_message.packet_id() == 2) << "packet id should be: " << last_message.packet_id();
  ut::expect(last_message.topic() == "mqtt-broadcaster-tests/def/bool/test_signal")
      << "topic should be: " << last_message.topic();
  ut::expect(last_message.payload()[0] == "true") << "payload should be: " << last_message.payload()[0];
  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos should be: " << last_message.opts().get_qos();
}

// This function tests adding a signal while the program is running to see weather the signal is added correctly and values
// are sent from the added signal.
static auto add_signal_in_running(
    asio::io_context& ctx,
    std::shared_ptr<mock_mqtt_client> mqtt_client,
    tfc::ipc_ruler::ipc_manager_client_mock& ipc_client,
    tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock>& signal) -> void {
  ctx.run_for(timeout_duration);

  auto last_message = mqtt_client->get_last_message();

  ut::expect(last_message.packet_id() == 2) << last_message.packet_id();
  ut::expect(last_message.topic() == "mqtt-broadcaster-tests/def/bool/test_signal")
      << "topic should be: " << last_message.topic();
  ut::expect(last_message.payload()[0] == "true") << "payload should be: " << last_message.payload()[0];
  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos should be: " << last_message.opts().get_qos();

  signal.send(false);
  ctx.run_for(timeout_duration);
  last_message = mqtt_client->get_last_message();

  ut::expect(last_message.packet_id() == 3) << "packet id should be: " << last_message.packet_id();
  ut::expect(last_message.topic() == "mqtt-broadcaster-tests/def/bool/test_signal")
      << "topic should be: " << last_message.topic();
  ut::expect(last_message.payload()[0] == "false") << "payload should be: " << last_message.payload()[0];
  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos should be: " << last_message.opts().get_qos();

  ctx.run_for(timeout_duration);

  tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock> added_signal(
      ctx, ipc_client, "added_signal", "description2");

  std::cout << "after adding\n";

  ctx.run_for(timeout_duration);

  added_signal.send(true);

  ctx.run_for(timeout_duration);

  last_message = mqtt_client->get_last_message();

  // the packet ID should be 6, 3 before and then both signals are reloaded, sending their initial values on reload, and then
  // the added signal sends another value
  ut::expect(last_message.packet_id() == 6) << "packet id should be: " << last_message.packet_id();
  ut::expect(last_message.topic() == "mqtt-broadcaster-tests/def/bool/added_signal")
      << "topic should be: " << last_message.topic();
  ut::expect(last_message.payload()[0] == "true") << "payload should be: " << last_message.payload()[0];
  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos should be: " << last_message.opts().get_qos();
}

// This function tests if the program works correctly when the MQTT broker goes down and comes back up.
static auto mqtt_broker_goes_down(
    asio::io_context& ctx,
    std::shared_ptr<mock_mqtt_client> mqtt_client,
    tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock> signal) -> void {
  ctx.run_for(timeout_duration);

  auto last_message = mqtt_client->get_last_message();

  ut::expect(last_message.packet_id() == 6) << last_message.packet_id();
  ut::expect(last_message.topic() == "mqtt-broadcaster-tests/def/bool/added_signal")
      << "topic should be: " << last_message.topic();
  ut::expect(last_message.payload()[0] == "true") << "payload should be: " << last_message.payload()[0];
  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos should be: " << last_message.opts().get_qos();

  signal.send(true);
  ctx.run_for(timeout_duration);

  last_message = mqtt_client->get_last_message();

  ut::expect(last_message.packet_id() == 7) << "packet id should be: " << last_message.packet_id();
  ut::expect(last_message.topic() == "mqtt-broadcaster-tests/def/bool/test_signal")
      << "topic should be: " << last_message.topic();
  ut::expect(last_message.payload()[0] == "true") << "payload should be: " << last_message.payload()[0];
  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos should be: " << last_message.opts().get_qos();

  ctx.run_for(timeout_duration);
  mqtt_client->set_online(false);
  ctx.run_for(timeout_duration);
  signal.send(false);
  ctx.run_for(timeout_duration);

  last_message = mqtt_client->get_last_message();

  ut::expect(last_message.topic() == "mqtt-broadcaster-tests/def/bool/test_signal")
      << "topic is: " << last_message.topic() << " when it should be: mqtt-broadcaster-tests/def/bool/test_signal";
  ut::expect(last_message.payload()[0] == "true")
      << "payload is: " << last_message.payload()[0] << " when it should be: true";
  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos is: " << last_message.opts().get_qos() << " when it should be: at_least_once";

  mqtt_client->set_online(true);
  ctx.run_for(timeout_duration);

  last_message = mqtt_client->get_last_message();

  ut::expect(last_message.topic() == "mqtt-broadcaster-tests/def/bool/test_signal")
      << "topic is: " << last_message.topic() << " when it should be: mqtt-broadcaster-tests/def/bool/test_signal";

  ut::expect(last_message.payload()[0] == "true")
      << "payload is: " << last_message.payload()[0] << " when it should be: false";

  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos is: " << last_message.opts().get_qos() << " when it should be: at_least_once";
}

auto main(int argc, char* argv[]) -> int {
  tfc::base::init(argc, argv);

  std::string mqtt_host{ "localhost" };
  std::string mqtt_port{ "1883" };
  std::string mqtt_username{ "" };
  std::string mqtt_password{ "" };

  boost::asio::io_context ctx{};

  tfc::ipc_ruler::ipc_manager_client_mock ipc_client;

  tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock> signal(
      ctx, ipc_client, "test_signal", "description2");

  const std::shared_ptr<mock_mqtt_client> mqtt_client =
      std::make_shared<mock_mqtt_client>(async_mqtt::protocol_version::v5, ctx.get_executor());

  tfc::confman::stub_config<config> cfg{ ctx, "mqtt_broadcaster",
                                         config{ ._allowed_topics = tfc::confman::observable<std::vector<std::string>>{} } };

  cfg.access()._allowed_topics = { "test_signal", "added_signal" };

  const mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client_mock&, mock_mqtt_client, config, tfc::confman::stub_config>
      application(ctx, std::move(mqtt_host), std::move(mqtt_port), std::move(mqtt_username), std::move(mqtt_password),
                  ipc_client, mqtt_client, cfg);

  "Sending a single value"_test = [&] {
    std::cout << "First test\n";
    send_simple_value(ctx, mqtt_client, signal);
  };

  "Adding a signal while the program is running"_test = [&] {
    std::cout << "Second test\n";
    add_signal_in_running(ctx, mqtt_client, ipc_client, signal);
  };

  "MQTT broker goes down"_test = [&] {
    std::cout << "Third test\n";
    mqtt_broker_goes_down(ctx, mqtt_client, signal);
  };

  return 0;
}
