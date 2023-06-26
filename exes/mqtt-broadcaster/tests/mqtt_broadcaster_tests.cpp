#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>
#include <boost/ut.hpp>
#include <tfc/ipc.hpp>
#include <tfc/mocks/confman/file_storage.hpp>
#include "mqtt_broadcaster.hpp"

namespace ut = boost::ut;

using boost::ut::operator""_test;
using boost::ut::operator/;

class mock_matcher {
public:
  mock_matcher([[maybe_unused]] sdbusplus::asio::connection& connection,
               std::string rule,
               std::function<void(sdbusplus::message::message&)> handler)
      : rule_(std::move(rule)), handler_(std::move(handler)) {}

private:
  std::string rule_;
  std::function<void(sdbusplus::message::message&)> handler_;
};

class result {
public:
  explicit result(std::string& message) : message_(message) {}

  explicit operator bool() const { return message_ != "Success"; }

  [[nodiscard]] auto message() -> std::string { return message_; }

private:
  std::string message_;
};

class mock_mqtt_client {
public:
  mock_mqtt_client(async_mqtt::protocol_version, const boost::asio::io_context::executor_type& executor)
      : strand_(executor), socket_(executor.context()), online_(true) {}

  auto strand() -> boost::asio::strand<boost::asio::io_context::executor_type>& { return strand_; }

  auto send(async_mqtt::v3_1_1::publish_packet packet, const boost::asio::use_awaitable_t<>&)
      -> boost::asio::awaitable<result> {
    if (online_) {
      messages_.push_back(packet);
      std::string message = "Success";
      co_return result{ message };
    } else {
      std::string message = "Failure";
      co_return result{ message };
    }
  }

  auto get_last_message() -> async_mqtt::v3_1_1::publish_packet {
    auto last_message = messages_.back();
    return last_message;
  }

  // Overload for connect_packet
  auto send([[maybe_unused]] async_mqtt::v3_1_1::connect_packet packet, const boost::asio::use_awaitable_t<>&)
      -> boost::asio::awaitable<result> {
    if (online_) {
      std::string message = "Success";
      co_return result{ message };
    } else {
      std::string message = "Failure";
      co_return result{ message };
    }
  }

  auto acquire_unique_packet_id() -> std::optional<uint16_t> {
    packet_id_++;
    return packet_id_;
  }

  static auto close(boost::asio::use_awaitable_t<>) -> boost::asio::awaitable<void> { co_return; }

  auto recv(boost::asio::use_awaitable_t<>) -> boost::asio::awaitable<async_mqtt::packet_variant> {
    if (online_) {
      co_return async_mqtt::v3_1_1::connack_packet{ true, async_mqtt::connect_return_code::accepted };
    } else {
      co_return async_mqtt::v3_1_1::connack_packet{ false, async_mqtt::connect_return_code::accepted };
    }
  }

  auto next_layer() -> boost::asio::ip::tcp::socket& { return socket_; }

  auto set_online(bool online) -> void { this->online_ = online; }

private:
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
  std::vector<async_mqtt::v3_1_1::publish_packet> messages_;
  boost::asio::ip::tcp::socket socket_;
  uint16_t packet_id_ = 0;
  bool online_;
};

auto send_simple_value(int timeout_in_ms) -> void {
  boost::asio::io_context ctx{};

  const std::string mqtt_host{ "localhost" };
  const std::string mqtt_port{ "1883" };

  tfc::ipc_ruler::ipc_manager_client_mock ipc_client;

  tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock> sig(ctx, ipc_client, "test_signal",
                                                                                              "description");

  const std::shared_ptr<mock_mqtt_client> mqtt_client =
      std::make_shared<mock_mqtt_client>(async_mqtt::protocol_version::v3_1_1, ctx.get_executor());

  const mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client_mock&, mock_matcher, mock_mqtt_client> application(
      ctx, mqtt_host, mqtt_port, ipc_client, mqtt_client);

  auto last_message = mqtt_client->get_last_message();

  ut::expect(last_message.packet_id() == 1) << last_message.packet_id();
  ut::expect(last_message.topic() == "test_topic") << "topic should be: " << last_message.topic();
  ut::expect(last_message.payload()[0] == "test_payload") << "payload should be: " << last_message.payload()[0];
  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos should be: " << last_message.opts().get_qos();

  ctx.run_for(std::chrono::milliseconds(timeout_in_ms));

  last_message = mqtt_client->get_last_message();

  ut::expect(last_message.packet_id() == 2) << last_message.packet_id();
  ut::expect(last_message.topic() == "mqtt-broadcaster-tests/def/bool/test_signal")
      << "topic should be: " << last_message.topic();
  ut::expect(last_message.payload()[0] == "false") << "payload should be: " << last_message.payload()[0];
  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos should be: " << last_message.opts().get_qos();

  sig.send(true);
  ctx.run_for(std::chrono::milliseconds(timeout_in_ms));
  last_message = mqtt_client->get_last_message();

  ut::expect(last_message.packet_id() == 3) << "packet id should be: " << last_message.packet_id();
  ut::expect(last_message.topic() == "mqtt-broadcaster-tests/def/bool/test_signal")
      << "topic should be: " << last_message.topic();
  ut::expect(last_message.payload()[0] == "true") << "payload should be: " << last_message.payload()[0];
  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos should be: " << last_message.opts().get_qos();
}

auto add_signal_in_running(int timeout_in_ms) -> void {
  boost::asio::io_context ctx{};

  const std::string mqtt_host{ "localhost" };
  const std::string mqtt_port{ "1883" };

  tfc::ipc_ruler::ipc_manager_client_mock ipc_client;

  tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock> sig(ctx, ipc_client, "test_signal",
                                                                                              "description");

  const std::shared_ptr<mock_mqtt_client> mqtt_client =
      std::make_shared<mock_mqtt_client>(async_mqtt::protocol_version::v3_1_1, ctx.get_executor());

  const mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client_mock&, mock_matcher, mock_mqtt_client> application(
      ctx, mqtt_host, mqtt_port, ipc_client, mqtt_client);

  auto last_message = mqtt_client->get_last_message();

  ut::expect(last_message.packet_id() == 1) << last_message.packet_id();
  ut::expect(last_message.topic() == "test_topic") << "topic should be: " << last_message.topic();
  ut::expect(last_message.payload()[0] == "test_payload") << "payload should be: " << last_message.payload()[0];
  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos should be: " << last_message.opts().get_qos();

  ctx.run_for(std::chrono::milliseconds(timeout_in_ms));

  last_message = mqtt_client->get_last_message();

  ut::expect(last_message.packet_id() == 2) << last_message.packet_id();
  ut::expect(last_message.topic() == "mqtt-broadcaster-tests/def/bool/test_signal")
      << "topic should be: " << last_message.topic();
  ut::expect(last_message.payload()[0] == "false") << "payload should be: " << last_message.payload()[0];
  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos should be: " << last_message.opts().get_qos();

  sig.send(true);
  ctx.run_for(std::chrono::milliseconds(timeout_in_ms));
  last_message = mqtt_client->get_last_message();

  ut::expect(last_message.packet_id() == 3) << "packet id should be: " << last_message.packet_id();
  ut::expect(last_message.topic() == "mqtt-broadcaster-tests/def/bool/test_signal")
      << "topic should be: " << last_message.topic();
  ut::expect(last_message.payload()[0] == "true") << "payload should be: " << last_message.payload()[0];
  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos should be: " << last_message.opts().get_qos();

  ctx.run_for(std::chrono::milliseconds(timeout_in_ms));

  tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock> sig2(
      ctx, ipc_client, "test_signal2", "description2");

  ctx.run_for(std::chrono::milliseconds(timeout_in_ms));

  sig2.send(true);

  ctx.run_for(std::chrono::milliseconds(timeout_in_ms));

  last_message = mqtt_client->get_last_message();
  ut::expect(last_message.packet_id() == 3) << "packet id should be: " << last_message.packet_id();
  ut::expect(last_message.topic() == "mqtt-broadcaster-tests/def/bool/test_signal")
      << "topic should be: " << last_message.topic();
  ut::expect(last_message.payload()[0] == "true") << "payload should be: " << last_message.payload()[0];
  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos should be: " << last_message.opts().get_qos();

}

auto mqtt_broker_goes_down(int timeout_in_ms) {

  boost::asio::io_context ctx{};

  const std::string mqtt_host{ "localhost" };
  const std::string mqtt_port{ "1883" };

  tfc::ipc_ruler::ipc_manager_client_mock ipc_client;

  tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock> sig(ctx, ipc_client, "test_signal",
                                                                                              "description");

  const std::shared_ptr<mock_mqtt_client> mqtt_client =
      std::make_shared<mock_mqtt_client>(async_mqtt::protocol_version::v3_1_1, ctx.get_executor());

  const mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client_mock&, mock_matcher, mock_mqtt_client> application(
      ctx, mqtt_host, mqtt_port, ipc_client, mqtt_client);

  auto last_message = mqtt_client->get_last_message();

  ut::expect(last_message.packet_id() == 1) << last_message.packet_id();
  ut::expect(last_message.topic() == "test_topic") << "topic should be: " << last_message.topic();
  ut::expect(last_message.payload()[0] == "test_payload") << "payload should be: " << last_message.payload()[0];
  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos should be: " << last_message.opts().get_qos();

  ctx.run_for(std::chrono::milliseconds(timeout_in_ms));

  last_message = mqtt_client->get_last_message();

  ut::expect(last_message.packet_id() == 2) << last_message.packet_id();
  ut::expect(last_message.topic() == "mqtt-broadcaster-tests/def/bool/test_signal")
      << "topic should be: " << last_message.topic();
  ut::expect(last_message.payload()[0] == "false") << "payload should be: " << last_message.payload()[0];
  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos should be: " << last_message.opts().get_qos();

  sig.send(true);
  ctx.run_for(std::chrono::milliseconds(timeout_in_ms));

  last_message = mqtt_client->get_last_message();

  ut::expect(last_message.packet_id() == 3) << "packet id should be: " << last_message.packet_id();
  ut::expect(last_message.topic() == "mqtt-broadcaster-tests/def/bool/test_signal")
      << "topic should be: " << last_message.topic();
  ut::expect(last_message.payload()[0] == "true") << "payload should be: " << last_message.payload()[0];
  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos should be: " << last_message.opts().get_qos();

  ctx.run_for(std::chrono::milliseconds(timeout_in_ms));
  mqtt_client->set_online(false);
  ctx.run_for(std::chrono::milliseconds(timeout_in_ms));
  sig.send(false);
  ctx.run_for(std::chrono::milliseconds(timeout_in_ms));

  last_message = mqtt_client->get_last_message();

  ut::expect(last_message.packet_id() == 3) << "packet id should be: " << last_message.packet_id();
  ut::expect(last_message.topic() == "mqtt-broadcaster-tests/def/bool/test_signal")
      << "topic should be: " << last_message.topic();
  ut::expect(last_message.payload()[0] == "true") << "payload should be: " << last_message.payload()[0];
  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos should be: " << last_message.opts().get_qos();

  mqtt_client->set_online(true);
  ctx.run_for(std::chrono::milliseconds(timeout_in_ms));
  ctx.run_for(std::chrono::milliseconds(timeout_in_ms));
  ctx.run_for(std::chrono::milliseconds(timeout_in_ms));
  ctx.run_for(std::chrono::milliseconds(timeout_in_ms));
  ctx.run_for(std::chrono::milliseconds(timeout_in_ms));

  ut::expect(last_message.packet_id() == 4) << "packet id should be: " << last_message.packet_id();
  ut::expect(last_message.topic() == "mqtt-broadcaster-tests/def/bool/test_signal")
      << "topic should be: " << last_message.topic();
  ut::expect(last_message.payload()[0] == "false") << "payload should be: " << last_message.payload()[0];
  ut::expect(last_message.opts().get_qos() == async_mqtt::qos::at_least_once)
      << "qos should be: " << last_message.opts().get_qos();


}


auto main(int argc, char* argv[]) -> int {
  try {
    const int timeout_in_ms = 100;

    tfc::base::init(argc, argv);
    send_simple_value(timeout_in_ms);
    add_signal_in_running(timeout_in_ms);
    mqtt_broker_goes_down(timeout_in_ms);

    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Exception caught: " << e.what() << '\n';
    return 1;  // or any other non-zero exit code
  } catch (...) {
    std::cerr << "Unknown exception caught\n";
    return 2;  // or any other non-zero exit code
  }
}
