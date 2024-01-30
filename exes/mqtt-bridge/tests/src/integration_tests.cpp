#include <iostream>

#include <sparkplug_b/sparkplug_b.pb.h>
#include <async_mqtt/all.hpp>
#include <async_mqtt/broker/broker.hpp>
#include <async_mqtt/broker/endpoint_variant.hpp>
#include <boost/asio.hpp>
#include <boost/ut.hpp>

#include <config/bridge_mock.hpp>
#include <tfc/progbase.hpp>
#include "../inc/endpoint_mock.hpp"

#include <tfc/ipc/details/dbus_client_iface_mock.hpp>

#include <run.hpp>

namespace ut = boost::ut;
using ut::operator""_test;
using ut::expect;

namespace asio = boost::asio;
namespace am = async_mqtt;

struct message {
  std::string topic;
  std::string payload;
};

class mqtt_broker {
public:
  mqtt_broker(asio::io_context& io_ctx)
      : io_ctx_(io_ctx), mqtt_endpoint_(asio::ip::tcp::v4(), 1883), mqtt_acceptor_(io_ctx_, mqtt_endpoint_),
        broker_(io_ctx_) {
    setup_async_connect();
  }

private:
  void setup_async_connect() {
    mqtt_async_accept_ = [this] {
      auto endpoint = am::endpoint<am::role::server, am::protocol::mqtt>::create(am::protocol_version::undetermined,
                                                                                 io_ctx_.get_executor());

      auto& lowest_layer = endpoint->lowest_layer();
      mqtt_acceptor_.async_accept(lowest_layer, [this, endpoint](boost::system::error_code const& ec) mutable {
        if (!ec) {
          broker_.handle_accept(epv_t{ std::move(endpoint) });
          mqtt_async_accept_();
        } else {
          std::cerr << "TCP accept error: " << ec.message() << std::endl;
        }
      });
    };

    mqtt_async_accept_();
  }

  asio::io_context& io_ctx_;
  asio::ip::tcp::endpoint mqtt_endpoint_;
  asio::ip::tcp::acceptor mqtt_acceptor_;
  std::function<void()> mqtt_async_accept_;
  using epv_t = am::endpoint_variant<am::role::server, am::protocol::mqtt>;
  am::broker<epv_t> broker_;
};

class mqtt_client {
public:
  mqtt_client(asio::io_context& io_ctx, std::vector<message>& messages, std::string& topic)
      : messages_(messages),
        // : io_ctx_(io_ctx),
        // messages_(messages),
        amep_(am::endpoint<am::role::client, am::protocol::mqtt>::create(am::protocol_version::v5, io_ctx.get_executor())),
        resolver_(io_ctx), topic_(topic) {
    connect("127.0.0.1", "1883");
  }

  void connect(const std::string& host, const std::string& port) {
    resolver_.async_resolve(host, port, [&](boost::system::error_code ec, asio::ip::tcp::resolver::results_type eps) {
      if (ec)
        return;
      handle_resolve(eps);
    });
  }

private:
  void handle_resolve(asio::ip::tcp::resolver::results_type eps) {
    asio::async_connect(amep_->next_layer(), eps, [&](boost::system::error_code, asio::ip::tcp::endpoint /*unused*/) {
      amep_->send(
          am::v5::connect_packet{
              true,
              0x1234,
              am::allocate_buffer("cid2"),
              am::nullopt,
              am::nullopt,
              am::nullopt,
          },
          [&](am::system_error const&) {
            amep_->recv([&](am::packet_variant pv) {
              pv.visit(am::overload{
                  [&](am::v5::connack_packet const&) {
                    amep_->send(
                        am::v5::subscribe_packet{ *amep_->acquire_unique_packet_id(),
                                                  { { am::allocate_buffer(topic_), am::qos::at_most_once } } },
                        [&](am::system_error const&) {
                          amep_->recv([&](am::packet_variant pvvv) {
                            pvvv.visit(am::overload{
                                [&](am::v5::suback_packet const&) {
                                  auto recv_handler = std::make_shared<std::function<void(am::packet_variant pv)>>();
                                  *recv_handler = [&, recv_handler](am::packet_variant pvv) {
                                    pvv.visit(am::overload{ [&](am::v5::publish_packet const& p) {
                                                             for (auto& payload : p.payload()) {
                                                               client_log.trace("\n\n\n\n");
                                                               client_log.trace("received message on topic: {}, payload: {}",
                                                                                p.topic().data(), payload.data());
                                                               client_log.trace("\n\n\n\n");
                                                               messages_.emplace_back(p.topic().data(), payload.data());
                                                             }
                                                             amep_->recv(*recv_handler);
                                                           },
                                                            [](auto const&) {} });
                                  };
                                  amep_->recv(*recv_handler);
                                },
                                [](auto const&) {} });
                          });
                        });
                  },
                  [](auto const&) {} });
            });
          });
    });
  }

  // asio::io_context& io_ctx_;
  std::vector<message>& messages_;
  decltype(am::endpoint<am::role::client, am::protocol::mqtt>::create(am::protocol_version::v5)) amep_;
  asio::ip::tcp::resolver resolver_;
  std::string& topic_;
  tfc::logger::logger client_log{ "client" };
};

auto main(int argc, char* argv[]) -> int {
  tfc::base::init(argc, argv);

  asio::io_context io_ctx{};

  // "testing if nbirth is sent, no signals"_test = [&]() {
  //   std::vector<message> messages;

  //   mqtt_broker broker{ io_ctx };

  //   io_ctx.run_for(std::chrono::seconds{ 1 });

  //   std::string nbirth_topic = "spBv1.0/tfc_unconfigured_group_id/NBIRTH/tfc_unconfigured_node_id";

  //   mqtt_client cli{ io_ctx, messages, nbirth_topic };

  //   io_ctx.run_for(std::chrono::seconds{ 1 });

  //   tfc::mqtt::run<tfc::mqtt::config::bridge_mock, tfc::mqtt::client_semi_normal, tfc::ipc_ruler::ipc_manager_client>
  //       running{ io_ctx };

  //   co_spawn(io_ctx, running.start(), asio::detached);

  //   io_ctx.run_for(std::chrono::seconds{ 1 });

  //   expect(messages.size() == 1);

  //   if (messages.size() >= 1) {
  //     expect(messages[0].topic == nbirth_topic);

  //     org::eclipse::tahu::protobuf::Payload payload;

  //     payload.ParseFromString(messages[0].payload);

  //     expect(payload.metrics_size() == 1) << "metrics size: " << payload.metrics_size();
  //     expect(payload.metrics()[0].name() == "Node Control/Rebirth");
  //     expect(payload.metrics()[0].datatype() == 11);
  //     expect(!payload.metrics()[0].is_historical());
  //     expect(!payload.metrics()[0].is_transient());
  //     expect(!payload.metrics()[0].is_null());
  //     expect(!payload.metrics()[0].boolean_value());
  //   }
  // };

  // --------------------------------------------------
  // "test sending value on signal, see if it sends correct initial value in NBIRTH"_test = [&]() {
  std::vector<message> messages;

  tfc::logger::logger log{ "log" };

  mqtt_broker broker{ io_ctx };

  io_ctx.run_for(std::chrono::seconds{ 1 });

  // std::string nbirth_topic = "spBv1.0/tfc_unconfigured_group_id/NBIRTH/tfc_unconfigured_node_id";
  std::string ndata_topic = "spBv1.0/tfc_unconfigured_group_id/NDATA/tfc_unconfigured_node_id";

  // mqtt_client nbirth_cli{ io_ctx, messages, nbirth_topic };
  mqtt_client ndata_cli{ io_ctx, messages, ndata_topic };

  io_ctx.run_for(std::chrono::seconds{ 1 });

  tfc::ipc_ruler::ipc_manager_client_mock ipc_client{ io_ctx };

  tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&> sig{ io_ctx, ipc_client, "test" };

  io_ctx.run_for(std::chrono::seconds{ 1 });

  sig.send(true);

  for (auto& signal : ipc_client.signals_) {
    std::cout << signal.name << std::endl;
    log.trace("signal name: {}", signal.name);
  }

  io_ctx.run_for(std::chrono::seconds{ 1 });

  tfc::mqtt::run<tfc::mqtt::config::bridge_mock, tfc::mqtt::client_semi_normal, tfc::ipc_ruler::ipc_manager_client_mock&>
      running{ io_ctx, ipc_client };

  co_spawn(io_ctx, running.start(), asio::detached);

  io_ctx.run_for(std::chrono::seconds{ 1 });
  sig.send(false);
  io_ctx.run_for(std::chrono::seconds{ 1 });
  sig.send(true);
  io_ctx.run_for(std::chrono::seconds{ 1 });

  expect(messages.size() == 3) << "messages size: " << messages.size();

  if (messages.size() >= 3) {
    expect(messages[0].topic == ndata_topic);

    org::eclipse::tahu::protobuf::Payload payload;

    payload.ParseFromString(messages[0].payload);

    //  metrics {
    //    name: "integration_tests/def/bool/test"
    //    timestamp: 1706632055709
    //    datatype: 11
    //    metadata {
    //      description: ""
    //    }
    //    boolean_value: true
    //  }
    //  seq: 1

    expect(payload.metrics_size() == 1) << "metrics size: " << payload.metrics_size();
    expect(payload.metrics()[0].name() == "integration_tests/def/bool/test");
    expect(payload.metrics()[0].datatype() == 11);
    expect(false) << payload.debug
    expect(payload.metrics()[0].has_boolean_value());
    expect(payload.metrics()[0].boolean_value());

    // expect(payload.metrics_size() == 1) << "metrics size: " << payload.metrics_size();
    //  expect(payload.metrics()[1].name() == "integration_tests/def/bool/test");
    //  expect(payload.metrics()[1].datatype() == 11);
    //  expect(!payload.metrics()[1].is_historical());
    //  expect(!payload.metrics()[1].is_transient());
    //  expect(!payload.metrics()[1].is_null());
    //  expect(!payload.metrics()[1].boolean_value());
  }

  // return 0;
}
