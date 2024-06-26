#include <expected>
#include <iostream>
#include <tuple>

#include <tfc/utils/pragmas.hpp>

#include <sparkplug_b/sparkplug_b.pb.h>
#include <async_mqtt/all.hpp>
// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wshadow-uncaptured-local)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wshadow)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wshadow-field-in-constructor)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wdocumentation)
// clang-format on
#include <broker/broker.hpp>
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
#include <boost/ut.hpp>

#include <tfc/ipc/details/dbus_client_iface_mock.hpp>
#include <tfc/progbase.hpp>

#include <client.hpp>
#include <config/bridge_mock.hpp>
#include <constants.hpp>
#include <endpoint.hpp>
#include <run.hpp>
#include "../inc/endpoint_mock.hpp"

namespace ut = boost::ut;
using ut::operator""_test;
using ut::expect;

namespace asio = boost::asio;

class mqtt_broker {
public:
  explicit mqtt_broker(asio::io_context& io_ctx) : io_ctx_(io_ctx) { mqtt_async_accept(); }

  auto mqtt_async_accept() -> void {
    endpoint_ = async_mqtt::endpoint<async_mqtt::role::server, async_mqtt::protocol::mqtt>::create(
        async_mqtt::protocol_version::undetermined, io_ctx_.get_executor());

    auto& lowest_layer = endpoint_->lowest_layer();
    mqtt_acceptor_.async_accept(lowest_layer, [this](boost::system::error_code const& ec) mutable {
      if (!ec) {
        broker_.handle_accept(epv_t{ std::move(endpoint_) });
        mqtt_async_accept();
      } else {
        std::cerr << "TCP accept error: " << ec.message() << std::endl;
      }
    });
  }

  asio::io_context& io_ctx_;
  asio::ip::tcp::endpoint mqtt_endpoint_{ asio::ip::tcp::v4(), 1965 };
  asio::ip::tcp::acceptor mqtt_acceptor_{ io_ctx_, mqtt_endpoint_ };

  using epv_t = async_mqtt::endpoint_variant<async_mqtt::role::server, async_mqtt::protocol::mqtt>;
  async_mqtt::broker<epv_t> broker_{ io_ctx_ };

  decltype(async_mqtt::endpoint<async_mqtt::role::server, async_mqtt::protocol::mqtt>::create(
      async_mqtt::protocol_version::undetermined)) endpoint_;
};

class mqtt_client {
public:
  mqtt_client(asio::io_context& io_ctx, std::vector<async_mqtt::buffer>& messages, std::string& topic)
      : io_ctx_(io_ctx), messages_(messages), topic_(topic),
        amep_(async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>::create(
            async_mqtt::protocol_version::v5,
            io_ctx.get_executor())) {
    resolver_.async_resolve("127.0.0.1", "1965",
                            [this](boost::system::error_code, asio::ip::tcp::resolver::results_type eps) {
                              co_spawn(io_ctx_, handle_resolve(eps), asio::detached);
                            });
  }

  auto handle_resolve(asio::ip::tcp::resolver::results_type eps) -> asio::awaitable<void> {
    std::ignore = co_await async_connect(amep_->lowest_layer(), eps, asio::use_awaitable);
    co_await amep_->async_send(
        async_mqtt::v5::connect_packet{
            true,
            0x1234,
            "cid2",
            std::nullopt,
            std::nullopt,
            std::nullopt,
        },
        asio::use_awaitable);
    co_await amep_->async_recv(async_mqtt::filter::match, { async_mqtt::control_packet_type::connack }, asio::use_awaitable);
    co_await send_subscribe();
  }

  auto send_subscribe() -> asio::awaitable<void> {
    std::optional<async_mqtt::packet_id_type> packet_id = amep_->acquire_unique_packet_id();
    auto sub_packet = async_mqtt::v5::subscribe_packet{ packet_id.value(), { { topic_, async_mqtt::qos::at_most_once } } };
    co_await amep_->async_send(sub_packet, asio::use_awaitable);
    co_await amep_->async_recv(async_mqtt::filter::match, { async_mqtt::control_packet_type::suback }, asio::use_awaitable);
    co_await receive_publish_packets();
  }

  auto receive_publish_packets() -> asio::awaitable<void> {
    while (true) {
      auto p = co_await amep_->async_recv(async_mqtt::filter::match, { async_mqtt::control_packet_type::publish },
                                          asio::use_awaitable);
      async_mqtt::v5::publish_packet const& p2 = p.template get<async_mqtt::v5::publish_packet>();
      for (auto& payload : p2.payload_as_buffer()) {
        messages_.push_back(payload);
      }
    }
  }

  asio::io_context& io_ctx_;
  std::vector<async_mqtt::buffer>& messages_;
  std::string& topic_;

  decltype(async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>::create(
      async_mqtt::protocol_version::v5)) amep_;

  asio::ip::tcp::resolver resolver_{ io_ctx_ };
};

auto main(int argc, char* argv[]) -> int {
  tfc::base::init(argc, argv);

  /// NOTE: broker is running on port 1965 because that port is never used for anything

  "correct nbirth with no signals"_test = [&]() {
    asio::io_context io_ctx{};

    // start broker
    mqtt_broker broker{ io_ctx };
    io_ctx.run_for(std::chrono::milliseconds{ 5 });

    // start client
    std::string nbirth_topic = "spBv1.0/tfc_unconfigured_group_id/NBIRTH/tfc_unconfigured_node_id";
    std::vector<async_mqtt::buffer> messages;
    mqtt_client cli{ io_ctx, messages, nbirth_topic };
    io_ctx.run_for(std::chrono::milliseconds{ 5 });

    // start mock ipc client
    tfc::ipc_ruler::ipc_manager_client_mock ipc_client{ io_ctx };
    io_ctx.run_for(std::chrono::milliseconds{ 5 });

    // start mqtt bridge
    tfc::mqtt::run<tfc::mqtt::config::bridge_mock,
                   tfc::mqtt::client<tfc::mqtt::endpoint_client, tfc::mqtt::config::bridge_mock>,
                   tfc::ipc_ruler::ipc_manager_client_mock&>
        running{ io_ctx, ipc_client };
    co_spawn(io_ctx, running.start(), asio::detached);
    io_ctx.run_for(std::chrono::milliseconds{ 50 });

    expect(messages.size() == 1);

    org::eclipse::tahu::protobuf::Payload nbirth_message;
    nbirth_message.ParseFromArray(messages[0].data(), messages[0].size());
    expect(nbirth_message.metrics_size() == 2);
    expect(nbirth_message.has_seq());
    expect(nbirth_message.seq() == 0);

    // rebirth metric
    expect(nbirth_message.metrics()[0].name() == "Node Control/Rebirth");
    expect(nbirth_message.metrics()[0].datatype() == 11);
    expect(!nbirth_message.metrics()[0].is_historical());
    expect(!nbirth_message.metrics()[0].is_transient());
    expect(!nbirth_message.metrics()[0].is_null());
    expect(nbirth_message.metrics()[0].has_boolean_value());
    expect(!nbirth_message.metrics()[0].boolean_value());

    // bdSeq metric
    expect(nbirth_message.metrics()[1].name() == "bdSeq");
    expect(nbirth_message.metrics()[1].datatype() == 8);
    expect(nbirth_message.metrics()[1].has_long_value());
    expect(nbirth_message.metrics()[1].long_value() == 0);
  };

  "sending value on signal"_test = [&]() {
    asio::io_context io_ctx{};

    // start broker
    mqtt_broker broker2{ io_ctx };
    io_ctx.run_for(std::chrono::milliseconds{ 5 });

    // start client
    std::string ndata_topic = "spBv1.0/tfc_unconfigured_group_id/NDATA/tfc_unconfigured_node_id";
    std::vector<async_mqtt::buffer> messages2;
    mqtt_client ndata_cli{ io_ctx, messages2, ndata_topic };
    io_ctx.run_for(std::chrono::milliseconds{ 5 });

    // start mock ipc client
    tfc::ipc_ruler::ipc_manager_client_mock ipc_client2{ io_ctx };
    tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&> sig_b{ io_ctx, ipc_client2,
                                                                                                    "test" };
    tfc::ipc::signal<tfc::ipc::details::type_string, tfc::ipc_ruler::ipc_manager_client_mock&> sig_s{ io_ctx, ipc_client2,
                                                                                                      "test" };
    io_ctx.run_for(std::chrono::milliseconds{ 5 });

    // send values on signals before startup
    sig_b.send(true);
    io_ctx.run_for(std::chrono::milliseconds{ 5 });
    sig_s.send("Initial");
    io_ctx.run_for(std::chrono::milliseconds{ 5 });

    // start mqtt bridge
    tfc::mqtt::run<tfc::mqtt::config::bridge_mock,
                   tfc::mqtt::client<tfc::mqtt::endpoint_client, tfc::mqtt::config::bridge_mock>,
                   tfc::ipc_ruler::ipc_manager_client_mock&>
        running{ io_ctx, ipc_client2 };
    co_spawn(io_ctx, running.start(), asio::detached);
    io_ctx.run_for(std::chrono::milliseconds{ 50 });

    // send more values
    sig_b.send(false);
    io_ctx.run_for(std::chrono::milliseconds{ 5 });
    sig_s.send("number_2");
    io_ctx.run_for(std::chrono::milliseconds{ 5 });
    sig_b.send(true);
    io_ctx.run_for(std::chrono::milliseconds{ 5 });
    sig_s.send("number_3");
    io_ctx.run_for(std::chrono::milliseconds{ 5 });

    expect(messages2.size() == 6) << "messages.size() == " << messages2.size();

    org::eclipse::tahu::protobuf::Payload first_message;
    first_message.ParseFromArray(messages2[0].data(), messages2[0].size());
    expect(first_message.metrics()[0].name() == "mqtt_bridge_integration_tests/def/bool/test");
    expect(first_message.metrics()[0].datatype() == 11);
    expect(first_message.metrics()[0].has_boolean_value());
    expect(first_message.metrics()[0].boolean_value());

    org::eclipse::tahu::protobuf::Payload second_message;
    second_message.ParseFromArray(messages2[1].data(), messages2[1].size());
    expect(second_message.metrics()[0].name() == "mqtt_bridge_integration_tests/def/string/test");
    expect(second_message.metrics()[0].datatype() == 12);
    expect(second_message.metrics()[0].has_string_value());
    expect(second_message.metrics()[0].string_value() == "Initial");

    org::eclipse::tahu::protobuf::Payload third_message;
    third_message.ParseFromArray(messages2[2].data(), messages2[2].size());
    expect(third_message.metrics()[0].name() == "mqtt_bridge_integration_tests/def/bool/test");
    expect(third_message.metrics()[0].datatype() == 11);
    expect(third_message.metrics()[0].has_boolean_value());
    expect(!third_message.metrics()[0].boolean_value());

    org::eclipse::tahu::protobuf::Payload fourth_message;
    fourth_message.ParseFromArray(messages2[3].data(), messages2[3].size());
    expect(fourth_message.metrics()[0].name() == "mqtt_bridge_integration_tests/def/string/test");
    expect(fourth_message.metrics()[0].datatype() == 12);
    expect(fourth_message.metrics()[0].has_string_value());
    expect(fourth_message.metrics()[0].string_value() == "number_2");

    org::eclipse::tahu::protobuf::Payload fifth_message;
    fifth_message.ParseFromArray(messages2[4].data(), messages2[4].size());
    expect(fifth_message.metrics()[0].name() == "mqtt_bridge_integration_tests/def/bool/test");
    expect(fifth_message.metrics()[0].datatype() == 11);
    expect(fifth_message.metrics()[0].has_boolean_value());
    expect(fifth_message.metrics()[0].boolean_value());

    org::eclipse::tahu::protobuf::Payload sixth_message;
    sixth_message.ParseFromArray(messages2[5].data(), messages2[5].size());
    expect(sixth_message.metrics()[0].name() == "mqtt_bridge_integration_tests/def/string/test");
    expect(sixth_message.metrics()[0].datatype() == 12);
    expect(sixth_message.metrics()[0].has_string_value());
    expect(sixth_message.metrics()[0].string_value() == "number_3");
  };

  return 0;
}
