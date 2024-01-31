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

// struct message {
//   std::string topic = "";
//   std::vector<char> payload = {};
// };

// struct message_test {
//   // std::string topic = "";
//   async_mqtt::buffer payload;
// };

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
  mqtt_client(asio::io_context& io_ctx,
              std::vector<async_mqtt::buffer>& messages,
              // std::vector<message_test>& message_tests,
              std::string& topic)
      : messages_(messages),  // messages_test_(message_tests),
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
  void recv_handler(am::packet_variant pvv) {
    pvv.visit(am::overload{ [&](am::v5::publish_packet const& p) {
                             for (auto& payload : p.payload()) {
                               //                            client_log.trace("\n\n\n\n");
                               //                            client_log.trace("received
                               //                            message on
                               //                            topic: {},
                               //                            payload:
                               //                            {}",
                               //                                             p.topic().data(), payload.data());
                               //                            client_log.trace("payload
                               //                            size: {}",
                               //                            p.payload().size());
                               //                            client_log.trace("\n\n\n\n");
                               //                            messages_.emplace_back();
                               //                            messages_[messages_.size()
                               //                            - 1].topic =
                               //                            p.topic();
                               //                            // There can
                               //                            be embeded
                               //                            null
                               //                            terminations
                               //                            in the
                               //                            binary data.
                               //                            //
                               //                            Constructing
                               //                            a simple
                               //                            std::string
                               //                            is not
                               //                            sufficient.
                               //                            std::copy(payload.begin(),
                               //                            payload.end(),
                               //                                      std::back_inserter(messages_[messages_.size()
                               //                                      -
                               //                                      1].payload));

                               // messages_test_.emplace_back(payload);
                               messages_.push_back(payload);
                             }
                             amep_->recv([this](auto&& pack) {
                               recv_handler(std::forward<decltype(pack)>(pack));
                             });
                           },
                            [](auto const&) {} });
  }
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
                            pvvv.visit(am::overload{ [&](am::v5::suback_packet const&) { amep_->recv([&](am::packet_variant pvv) {
                              recv_handler(pvv);
                            }); },
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
  std::vector<async_mqtt::buffer>& messages_;
  // std::vector<message>& messages_;
  // std::vector<message_test>& messages_test_;
  decltype(am::endpoint<am::role::client, am::protocol::mqtt>::create(am::protocol_version::v5)) amep_;
  asio::ip::tcp::resolver resolver_;
  std::string& topic_;
  tfc::logger::logger client_log{ "client" };
};

auto main(int argc, char* argv[]) -> int {
  tfc::base::init(argc, argv);

  // "testing if nbirth is sent, no signals"_test = [&]() {
  asio::io_context io_ctx{};

  //  // start broker
  //  mqtt_broker broker{ io_ctx };
  //  io_ctx.run_for(std::chrono::seconds{ 1 });

  //  // start client
  //  std::string nbirth_topic = "spBv1.0/tfc_unconfigured_group_id/NBIRTH/tfc_unconfigured_node_id";
  //  std::vector<async_mqtt::buffer> messages;
  //  mqtt_client cli{ io_ctx, messages, nbirth_topic };
  //  io_ctx.run_for(std::chrono::seconds{ 1 });

  //  // start mock ipc client
  //  tfc::ipc_ruler::ipc_manager_client_mock ipc_client{ io_ctx };
  //  io_ctx.run_for(std::chrono::seconds{ 1 });

  //  // start mqtt bridge
  //  tfc::mqtt::run<tfc::mqtt::config::bridge_mock, tfc::mqtt::client_semi_normal, tfc::ipc_ruler::ipc_manager_client_mock&>
  //      running{ io_ctx, ipc_client };
  //  co_spawn(io_ctx, running.start(), asio::detached);
  //  io_ctx.run_for(std::chrono::seconds{ 1 });

  //  expect(messages.size() == 1);

  //  org::eclipse::tahu::protobuf::Payload nbirth_message;
  //  nbirth_message.ParseFromArray(messages[0].data(), messages[0].size());
  //  expect(nbirth_message.metrics()[0].name() == "Node Control/Rebirth");
  //  expect(nbirth_message.metrics()[0].datatype() == 11);
  //  expect(!nbirth_message.metrics()[0].is_historical());
  //  expect(!nbirth_message.metrics()[0].is_transient());
  //  expect(!nbirth_message.metrics()[0].is_null());
  //  expect(!nbirth_message.metrics()[0].boolean_value());

  //  io_ctx.stop();

  // };

  // allow context to be cleared after previous test run
  // io_ctx.run_for(std::chrono::seconds{ 10 });

  tfc::logger::logger log{ "log" };

  // "test sending value on signal, see if it sends correct initial value in NBIRTH"_test = [&]() {
  //   asio::io_context io_ctx{};
  // start broker
  mqtt_broker broker{ io_ctx };
  // broker{ io_ctx };
  //   io_ctx.run_for(std::chrono::seconds{ 1 });

  //   // start client
  std::string ndata_topic = "spBv1.0/tfc_unconfigured_group_id/NDATA/tfc_unconfigured_node_id";
  std::vector<async_mqtt::buffer> messages;
  mqtt_client ndata_cli{ io_ctx, messages, ndata_topic };
  io_ctx.run_for(std::chrono::seconds{ 1 });

  //   // start mock ipc client
  tfc::ipc_ruler::ipc_manager_client_mock ipc_client{ io_ctx };
  tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&> sig_b{ io_ctx, ipc_client,
                                                                                                  "test" };
  tfc::ipc::signal<tfc::ipc::details::type_string, tfc::ipc_ruler::ipc_manager_client_mock&> sig_s{ io_ctx, ipc_client,
                                                                                                    "test" };
  //   io_ctx.run_for(std::chrono::seconds{ 1 });

  //   // send values on signals before startup
  //   sig_b.send(true);
  //   io_ctx.run_for(std::chrono::seconds{ 1 });
  //   sig_s.send("Initial");
  //   io_ctx.run_for(std::chrono::seconds{ 1 });

  //   // start mqtt bridge
  tfc::mqtt::run<tfc::mqtt::config::bridge_mock, tfc::mqtt::client_semi_normal, tfc::ipc_ruler::ipc_manager_client_mock&>
      running{ io_ctx, ipc_client };
  co_spawn(io_ctx, running.start(), asio::detached);
  io_ctx.run_for(std::chrono::milliseconds{ 100 });

  //   // send more values
  //   sig_b.send(false);
  //   io_ctx.run_for(std::chrono::seconds{ 1 });
  //   sig_s.send("number_2");
  //   io_ctx.run_for(std::chrono::seconds{ 1 });
  //   sig_b.send(true);
  //   io_ctx.run_for(std::chrono::seconds{ 1 });
  //   sig_s.send("number_3");
  io_ctx.run_for(std::chrono::seconds{ 1 });

  io_ctx.stop();

  std::cout << "io_ctx stopped: " << io_ctx.stopped() << std::endl;

  std::cout << "is same: " << (running.sp_interface_.strand().get_inner_executor() == io_ctx.get_executor()) << std::endl;

  //  expect(messages.size() == 6) << "messages.size() == " << messages.size();

  //  org::eclipse::tahu::protobuf::Payload first_message;
  //  first_message.ParseFromArray(messages[0].data(), messages[0].size());
  //  expect(first_message.metrics()[0].name() == "integration_tests/def/bool/test");
  //  expect(first_message.metrics()[0].datatype() == 11);
  //  expect(first_message.metrics()[0].has_boolean_value());
  //  expect(first_message.metrics()[0].boolean_value());

  //  org::eclipse::tahu::protobuf::Payload second_message;
  //  second_message.ParseFromArray(messages[1].data(), messages[1].size());
  //  expect(second_message.metrics()[0].name() == "integration_tests/def/string/test");
  //  expect(second_message.metrics()[0].datatype() == 12);
  //  expect(second_message.metrics()[0].has_string_value());
  //  expect(second_message.metrics()[0].string_value() == "Initial");

  //  org::eclipse::tahu::protobuf::Payload third_message;
  //  third_message.ParseFromArray(messages[2].data(), messages[2].size());
  //  expect(third_message.metrics()[0].name() == "integration_tests/def/bool/test");
  //  expect(third_message.metrics()[0].datatype() == 11);
  //  expect(third_message.metrics()[0].has_boolean_value());
  //  expect(!third_message.metrics()[0].boolean_value());

  //  org::eclipse::tahu::protobuf::Payload fourth_message;
  //  fourth_message.ParseFromArray(messages[3].data(), messages[3].size());
  //  expect(fourth_message.metrics()[0].name() == "integration_tests/def/string/test");
  //  expect(fourth_message.metrics()[0].datatype() == 12);
  //  expect(fourth_message.metrics()[0].has_string_value());
  //  expect(fourth_message.metrics()[0].string_value() == "number_2");

  //  org::eclipse::tahu::protobuf::Payload fifth_message;
  //  fifth_message.ParseFromArray(messages[4].data(), messages[4].size());
  //  expect(fifth_message.metrics()[0].name() == "integration_tests/def/bool/test");
  //  expect(fifth_message.metrics()[0].datatype() == 11);
  //  expect(fifth_message.metrics()[0].has_boolean_value());
  //  expect(fifth_message.metrics()[0].boolean_value());

  //  org::eclipse::tahu::protobuf::Payload sixth_message;
  //  sixth_message.ParseFromArray(messages[5].data(), messages[5].size());
  //  expect(sixth_message.metrics()[0].name() == "integration_tests/def/string/test");
  //  expect(sixth_message.metrics()[0].datatype() == 12);
  //  expect(sixth_message.metrics()[0].has_string_value());
  //  expect(sixth_message.metrics()[0].string_value() == "number_3");

  // };

  return 0;
}
