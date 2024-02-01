#include <sparkplug_b/sparkplug_b.pb.h>

#include <async_mqtt/all.hpp>
#include <async_mqtt/broker/broker.hpp>
#include <boost/ut.hpp>
#include <constants.hpp>

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

class mqtt_broker {
public:
  mqtt_broker(asio::io_context& io_ctx)
      : io_ctx_(io_ctx), mqtt_endpoint_(asio::ip::tcp::v4(), 1883), mqtt_acceptor_(io_ctx_, mqtt_endpoint_),
        broker_(io_ctx_) {
    setup_async_connect();
  }

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
  mqtt_client(asio::io_context& io_ctx, std::vector<async_mqtt::buffer>& messages, std::string& topic)
      : messages_(messages),
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

  void recv_handler(am::packet_variant pvv) {
    pvv.visit(am::overload{ [&](am::v5::publish_packet const& p) {
                             for (auto& payload : p.payload()) {
                               messages_.push_back(payload);
                             }
                             amep_->recv([this](auto&& pack) { recv_handler(std::forward<decltype(pack)>(pack)); });
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
                            pvvv.visit(am::overload{ [&](am::v5::suback_packet const&) {
                                                      amep_->recv([&](am::packet_variant pvv) { recv_handler(pvv); });
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

  std::vector<async_mqtt::buffer>& messages_;
  decltype(am::endpoint<am::role::client, am::protocol::mqtt>::create(am::protocol_version::v5)) amep_;
  asio::ip::tcp::resolver resolver_;
  std::string& topic_;
  tfc::logger::logger client_log{ "client" };
};

auto main(int argc, char* argv[]) -> int {
  tfc::base::init(argc, argv);

  //   "correct nbirth with no signals"_test = [&]() {
  //     asio::io_context io_ctx{};
  //
  //     // start broker
  //     mqtt_broker broker{ io_ctx };
  //     io_ctx.run_for(std::chrono::milliseconds{ 100 });
  //
  //     // start client
  //     std::string nbirth_topic = "spBv1.0/tfc_unconfigured_group_id/NBIRTH/tfc_unconfigured_node_id";
  //     std::vector<async_mqtt::buffer> messages;
  //     mqtt_client cli{ io_ctx, messages, nbirth_topic };
  //     io_ctx.run_for(std::chrono::milliseconds{ 100 });
  //
  //     // start mock ipc client
  //     tfc::ipc_ruler::ipc_manager_client_mock ipc_client{ io_ctx };
  //     io_ctx.run_for(std::chrono::milliseconds{ 100 });
  //
  //     // start mqtt bridge
  //     tfc::mqtt::run<tfc::mqtt::config::bridge_mock, tfc::mqtt::client_semi_normal,
  //     tfc::ipc_ruler::ipc_manager_client_mock&>
  //         running{ io_ctx, ipc_client };
  //     co_spawn(io_ctx, running.start(), asio::detached);
  //     io_ctx.run_for(std::chrono::milliseconds{ 100 });
  //
  //     expect(messages.size() == 1);
  //
  //     org::eclipse::tahu::protobuf::Payload nbirth_message;
  //     nbirth_message.ParseFromArray(messages[0].data(), messages[0].size());
  //     expect(nbirth_message.metrics_size() == 2);
  //     expect(nbirth_message.has_seq());
  //     expect(nbirth_message.seq() == 0);
  //
  //     // rebirth metric
  //     expect(nbirth_message.metrics()[0].name() == "Node Control/Rebirth");
  //     expect(nbirth_message.metrics()[0].datatype() == 11);
  //     expect(!nbirth_message.metrics()[0].is_historical());
  //     expect(!nbirth_message.metrics()[0].is_transient());
  //     expect(!nbirth_message.metrics()[0].is_null());
  //     expect(nbirth_message.metrics()[0].has_boolean_value());
  //     expect(!nbirth_message.metrics()[0].boolean_value());
  //
  //     // bdSeq metric
  //     expect(nbirth_message.metrics()[1].name() == "bdSeq");
  //     expect(nbirth_message.metrics()[1].datatype() == 8);
  //     expect(nbirth_message.metrics()[1].has_long_value());
  //     expect(nbirth_message.metrics()[1].long_value() == 0);
  //   };
  //
  //   "sending value on signal"_test = [&]() {
  //     asio::io_context io_ctx{};
  //
  //     // start broker
  //     mqtt_broker broker2{ io_ctx };
  //     io_ctx.run_for(std::chrono::milliseconds{ 100 });
  //
  //     // start client
  //     std::string ndata_topic = "spBv1.0/tfc_unconfigured_group_id/NDATA/tfc_unconfigured_node_id";
  //     std::vector<async_mqtt::buffer> messages2;
  //     mqtt_client ndata_cli{ io_ctx, messages2, ndata_topic };
  //     io_ctx.run_for(std::chrono::milliseconds{ 100 });
  //
  //     // start mock ipc client
  //     tfc::ipc_ruler::ipc_manager_client_mock ipc_client2{ io_ctx };
  //     tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&> sig_b{ io_ctx, ipc_client2,
  //                                                                                                     "test" };
  //     tfc::ipc::signal<tfc::ipc::details::type_string, tfc::ipc_ruler::ipc_manager_client_mock&> sig_s{ io_ctx,
  //     ipc_client2,
  //                                                                                                       "test" };
  //     io_ctx.run_for(std::chrono::milliseconds{ 100 });
  //
  //     // send values on signals before startup
  //     sig_b.send(true);
  //     io_ctx.run_for(std::chrono::milliseconds{ 100 });
  //     sig_s.send("Initial");
  //     io_ctx.run_for(std::chrono::milliseconds{ 100 });
  //
  //     // start mqtt bridge
  //     tfc::mqtt::run<tfc::mqtt::config::bridge_mock, tfc::mqtt::client_semi_normal,
  //     tfc::ipc_ruler::ipc_manager_client_mock&>
  //         running{ io_ctx, ipc_client2 };
  //     co_spawn(io_ctx, running.start(), asio::detached);
  //     io_ctx.run_for(std::chrono::milliseconds{ 100 });
  //
  //     // send more values
  //     sig_b.send(false);
  //     io_ctx.run_for(std::chrono::milliseconds{ 100 });
  //     sig_s.send("number_2");
  //     io_ctx.run_for(std::chrono::milliseconds{ 100 });
  //     sig_b.send(true);
  //     io_ctx.run_for(std::chrono::milliseconds{ 100 });
  //     sig_s.send("number_3");
  //     io_ctx.run_for(std::chrono::milliseconds{ 100 });
  //
  //     expect(messages2.size() == 6) << "messages.size() == " << messages2.size();
  //
  //     org::eclipse::tahu::protobuf::Payload first_message;
  //     first_message.ParseFromArray(messages2[0].data(), messages2[0].size());
  //     expect(first_message.metrics()[0].name() == "integration_tests/def/bool/test");
  //     expect(first_message.metrics()[0].datatype() == 11);
  //     expect(first_message.metrics()[0].has_boolean_value());
  //     expect(first_message.metrics()[0].boolean_value());
  //
  //     org::eclipse::tahu::protobuf::Payload second_message;
  //     second_message.ParseFromArray(messages2[1].data(), messages2[1].size());
  //     expect(second_message.metrics()[0].name() == "integration_tests/def/string/test");
  //     expect(second_message.metrics()[0].datatype() == 12);
  //     expect(second_message.metrics()[0].has_string_value());
  //     expect(second_message.metrics()[0].string_value() == "Initial");
  //
  //     org::eclipse::tahu::protobuf::Payload third_message;
  //     third_message.ParseFromArray(messages2[2].data(), messages2[2].size());
  //     expect(third_message.metrics()[0].name() == "integration_tests/def/bool/test");
  //     expect(third_message.metrics()[0].datatype() == 11);
  //     expect(third_message.metrics()[0].has_boolean_value());
  //     expect(!third_message.metrics()[0].boolean_value());
  //
  //     org::eclipse::tahu::protobuf::Payload fourth_message;
  //     fourth_message.ParseFromArray(messages2[3].data(), messages2[3].size());
  //     expect(fourth_message.metrics()[0].name() == "integration_tests/def/string/test");
  //     expect(fourth_message.metrics()[0].datatype() == 12);
  //     expect(fourth_message.metrics()[0].has_string_value());
  //     expect(fourth_message.metrics()[0].string_value() == "number_2");
  //
  //     org::eclipse::tahu::protobuf::Payload fifth_message;
  //     fifth_message.ParseFromArray(messages2[4].data(), messages2[4].size());
  //     expect(fifth_message.metrics()[0].name() == "integration_tests/def/bool/test");
  //     expect(fifth_message.metrics()[0].datatype() == 11);
  //     expect(fifth_message.metrics()[0].has_boolean_value());
  //     expect(fifth_message.metrics()[0].boolean_value());
  //
  //     org::eclipse::tahu::protobuf::Payload sixth_message;
  //     sixth_message.ParseFromArray(messages2[5].data(), messages2[5].size());
  //     expect(sixth_message.metrics()[0].name() == "integration_tests/def/string/test");
  //     expect(sixth_message.metrics()[0].datatype() == 12);
  //     expect(sixth_message.metrics()[0].has_string_value());
  //     expect(sixth_message.metrics()[0].string_value() == "number_3");
  //   };

  // writeable signals

  asio::io_context io_ctx{};

  // start broker
  mqtt_broker broker2{ io_ctx };
  io_ctx.run_for(std::chrono::milliseconds{ 100 });

  // start client
  std::string ndata_topic = "#";
  std::vector<async_mqtt::buffer> messages2;
  mqtt_client ndata_cli{ io_ctx, messages2, ndata_topic };
  io_ctx.run_for(std::chrono::milliseconds{ 100 });

  // start mock ipc client
  tfc::ipc_ruler::ipc_manager_client_mock ipc_client2{ io_ctx };
  io_ctx.run_for(std::chrono::milliseconds{ 100 });

  // start mqtt bridge
  tfc::mqtt::run<tfc::mqtt::config::bridge_mock, tfc::mqtt::client_semi_normal, tfc::ipc_ruler::ipc_manager_client_mock&>
      running{ io_ctx, ipc_client2 };
  co_spawn(io_ctx, running.start(), asio::detached);
  io_ctx.run_for(std::chrono::milliseconds{ 100 });

  for (auto& sig : ipc_client2.signals_) {
    std::cout << sig.name << std::endl;
  }

  auto slot_writeable = tfc::ipc::details::slot_callback<tfc::ipc::details::type_bool>::create(io_ctx, "writeable_signal");
  slot_writeable->connect("integration_tests.def.bool.test_writeable_signal", [](bool) {});
  io_ctx.run_for(std::chrono::milliseconds{ 100 });

  expect(!slot_writeable->value().has_value());

  org::eclipse::tahu::protobuf::Payload ncmd_message;
  auto new_metric = ncmd_message.add_metrics();
  new_metric->set_name("integration_tests.def.bool.test_writeable_signal");
  new_metric->set_boolean_value(true);

  std::string payload_string;
  ncmd_message.SerializeToString(&payload_string);

  std::string ncmd_topic = "spBv1.0/tfc_unconfigured_group_id/NCMD/tfc_unconfigured_node_id";

  //          packet_id_t packet_id,
  //        buffer topic_name,
  //        BufferSequence payloads,
  //        pub::opts pubopts,
  //        properties props = {}
  //   async_mqtt::v5::publish_packet ncmd_packet {
  //     async_mqtt::buffer{ std::string_view{ ncmd_topic } }, { async_mqtt::buffer{ std::string_view{ payload_string } } };
  //
  //     ndata_cli.amep_.send_message(ncmd_topic, payload_string, async_mqtt::qos::at_most_once);

  std::optional<uint16_t> p_id;

  //  if (qos != async_mqtt::qos::at_most_once) {
  //    p_id = endpoint_client_->acquire_unique_packet_id();
  //   } else {
  p_id = 0;
  // }

  auto pub_packet =
      async_mqtt::v5::publish_packet{ p_id.value(), async_mqtt::allocate_buffer(ncmd_topic),
                                      async_mqtt::allocate_buffer(payload_string), async_mqtt::qos::at_most_once };

  // auto send_error = co_await endpoint_client_->send(pub_packet, asio::use_awaitable);
  io_ctx.run_for(std::chrono::seconds{ 1 });

  std::cout << "sending packet" << std::endl;
  ndata_cli.amep_->send(pub_packet, [](async_mqtt::system_error const&) {});

  std::cout << "sending packet" << std::endl;
  io_ctx.run_for(std::chrono::seconds{ 1 });
  ndata_cli.amep_->send(pub_packet, [](async_mqtt::system_error const&) {});
  std::cout << "sending packet" << std::endl;
  io_ctx.run_for(std::chrono::seconds{ 10 });

  expect(slot_writeable->value().has_value());
  expect(slot_writeable->value().value());

  // Payload payload;
  //  payload.set_timestamp(timestamp_milliseconds().count());
  //  seq_ = 0;
  //  payload.set_seq(seq_);

  //  auto* node_rebirth = payload.add_metrics();
  //  node_rebirth->set_name(constants::rebirth_metric.data());
  //  node_rebirth->set_timestamp(timestamp_milliseconds().count());
  //  node_rebirth->set_datatype(11);
  //  node_rebirth->set_boolean_value(false);
  //  node_rebirth->set_is_transient(false);
  //  node_rebirth->set_is_historical(false);
  //  node_rebirth->set_is_null(false);

  //  auto* bd_seq_metric = payload.add_metrics();

  //  bd_seq_metric->set_name("bdSeq");
  //  bd_seq_metric->set_timestamp(timestamp_milliseconds().count());
  //  bd_seq_metric->set_datatype(8);
  //  bd_seq_metric->set_long_value(0);

  //  for (auto const& variable : variables_) {
  //    auto* variable_metric = payload.add_metrics();

  //    auto* metadata = variable_metric->mutable_metadata();
  //    metadata->set_description(variable.description);

  //    variable_metric->set_name(variable.name);
  //    variable_metric->set_datatype(variable.datatype);
  //    variable_metric->set_timestamp(timestamp_milliseconds().count());
  //    set_value_payload(variable_metric, variable.value, logger_);

  //    variable_metric->set_is_transient(false);
  //    variable_metric->set_is_historical(false);
  //  }

  //  logger_.trace("NBIRTH payload: \n {}", payload.DebugString());

  //                 asio::detached);

  return 0;
}
