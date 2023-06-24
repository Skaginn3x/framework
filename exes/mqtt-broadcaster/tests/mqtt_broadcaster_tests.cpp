#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>
#include <tfc/ipc.hpp>
#include <tfc/mocks/confman.hpp>
#include <tfc/mocks/confman/file_storage.hpp>
#include "mqtt_broadcaster.hpp"

class mock_matcher {
public:
  mock_matcher(sdbusplus::asio::connection& connection,
               std::string rule,
               std::function<void(sdbusplus::message::message&)> handler)
      : connection_(connection), rule_(std::move(rule)), handler_(std::move(handler)) {}

private:
  sdbusplus::asio::connection& connection_;
  std::string rule_;
  std::function<void(sdbusplus::message::message&)> handler_;
};

class result {
public:
  result(std::string& message) : message_(message) {}

  operator bool() const { return message_ == "okay"; }

  std::string message() const { return message_; }

private:
  std::string message_;
};

class mock_mqtt_client {
public:
  mock_mqtt_client(async_mqtt::protocol_version, boost::asio::io_context::executor_type executor)
      : strand_(executor), socket_(executor.context()) {}

  // mock_mqtt_client(boost::asio::io_context& context) : strand_(context.get_executor()), socket_(context) {}

  auto strand() -> boost::asio::strand<boost::asio::io_context::executor_type>& { return strand_; }

  boost::asio::awaitable<result> send(async_mqtt::v3_1_1::publish_packet packet, const boost::asio::use_awaitable_t<>&) {
    messages_.push_back(packet);
    std::string message = "okay";
    co_return result{ message };
  }

  // Overload for connect_packet
  boost::asio::awaitable<result> send([[maybe_unused]] async_mqtt::v3_1_1::connect_packet packet,
                                      const boost::asio::use_awaitable_t<>&) {
    std::string message = "okay";
    co_return result{ message };
  }

  auto acquire_unique_packet_id() -> std::optional<short unsigned int> { return 1; }

  // Add a close function
  boost::asio::awaitable<void> close(boost::asio::use_awaitable_t<>) { co_return; }

  boost::asio::awaitable<async_mqtt::packet_variant> recv(boost::asio::use_awaitable_t<>) {

    co_return async_mqtt::v3_1_1::connack_packet{ true, async_mqtt::connect_return_code::accepted };
  }

  auto next_layer() -> boost::asio::ip::tcp::socket& { return socket_; }

private:
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
  std::vector<async_mqtt::v3_1_1::publish_packet> messages_;
  boost::asio::ip::tcp::socket socket_;
};

auto main() -> int {
  boost::asio::io_context ctx{};

  std::string mqtt_host{ "localhost" };
  std::string mqtt_port{ "1883" };

  tfc::ipc_ruler::ipc_manager_client_mock ipc_client;

  tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock> sig(ctx, ipc_client, "test_signal",
                                                                                              "description");

  mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client_mock&, mock_matcher, mock_mqtt_client> application(
      ctx, mqtt_host, mqtt_port, ipc_client);

  ctx.run();

  return 0;
}
