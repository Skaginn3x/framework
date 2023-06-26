#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>
#include <tfc/ipc.hpp>
#include <tfc/mocks/confman/file_storage.hpp>
#include "mqtt_broadcaster.hpp"

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

  explicit operator bool() const { return message_ == "okay"; }

  [[nodiscard]] auto message() -> std::string { return message_; }

private:
  std::string message_;
};

class mock_mqtt_client {
public:
  mock_mqtt_client(async_mqtt::protocol_version, const boost::asio::io_context::executor_type& executor)
      : strand_(executor), socket_(executor.context()) {}

  // mock_mqtt_client(boost::asio::io_context& context) : strand_(context.get_executor()), socket_(context) {}

  auto strand() -> boost::asio::strand<boost::asio::io_context::executor_type>& { return strand_; }

  auto send(async_mqtt::v3_1_1::publish_packet packet, const boost::asio::use_awaitable_t<>&)
      -> boost::asio::awaitable<result> {
    messages_.push_back(packet);
    std::string message = "okay";
    co_return result{ message };
  }

  // Overload for connect_packet
  static auto send([[maybe_unused]] async_mqtt::v3_1_1::connect_packet packet, const boost::asio::use_awaitable_t<>&)
      -> boost::asio::awaitable<result> {
    std::string message = "okay";
    co_return result{ message };
  }

  static auto acquire_unique_packet_id() -> std::optional<uint16_t> { return 1; }

  // Add a close function
  static auto close(boost::asio::use_awaitable_t<>) -> boost::asio::awaitable<void> { co_return; }

  static auto recv(boost::asio::use_awaitable_t<>) -> boost::asio::awaitable<async_mqtt::packet_variant> {
    co_return async_mqtt::v3_1_1::connack_packet{ true, async_mqtt::connect_return_code::accepted };
  }

  auto next_layer() -> boost::asio::ip::tcp::socket& { return socket_; }

private:
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
  std::vector<async_mqtt::v3_1_1::publish_packet> messages_;
  boost::asio::ip::tcp::socket socket_;
};

auto main() -> int {
  try {
    boost::asio::io_context ctx{};

    const std::string mqtt_host{ "localhost" };
    const std::string mqtt_port{ "1883" };

    tfc::ipc_ruler::ipc_manager_client_mock ipc_client;

    const tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock> sig(
        ctx, ipc_client, "test_signal", "description");

    const mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client_mock&, mock_matcher, mock_mqtt_client> application(
        ctx, mqtt_host, mqtt_port, ipc_client);

    ctx.run();

    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Exception caught: " << e.what() << '\n';
    return 1;  // or any other non-zero exit code
  } catch (...) {
    std::cerr << "Unknown exception caught\n";
    return 2;  // or any other non-zero exit code
  }
}
