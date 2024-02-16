#ifdef __clang__

#include <expected>
#include <iostream>
#include <tuple>

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

class mqtt_broker {
public:
    explicit mqtt_broker(asio::io_context &io_ctx) : io_ctx_(io_ctx) { mqtt_async_accept(); }

    auto mqtt_async_accept() -> void {
        endpoint_ = async_mqtt::endpoint<async_mqtt::role::server, async_mqtt::protocol::mqtt>::create(
            async_mqtt::protocol_version::undetermined, io_ctx_.get_executor());

        auto &lowest_layer = endpoint_->lowest_layer();
        mqtt_acceptor_.async_accept(lowest_layer, [this](boost::system::error_code const &ec) mutable {
            if (!ec) {
                broker_.handle_accept(epv_t{std::move(endpoint_)});
                mqtt_async_accept();
            } else {
                std::cerr << "TCP accept error: " << ec.message() << std::endl;
            }
        });
    }

    asio::io_context &io_ctx_;
    asio::ip::tcp::endpoint mqtt_endpoint_{asio::ip::tcp::v4(), 1965};
    asio::ip::tcp::acceptor mqtt_acceptor_{io_ctx_, mqtt_endpoint_};

    using epv_t = async_mqtt::endpoint_variant<async_mqtt::role::server, async_mqtt::protocol::mqtt>;
    async_mqtt::broker<epv_t> broker_{io_ctx_};

    decltype(async_mqtt::endpoint<async_mqtt::role::server, async_mqtt::protocol::mqtt>::create(
        async_mqtt::protocol_version::undetermined)) endpoint_;
};

class mqtt_client {
public:
    mqtt_client(asio::io_context &io_ctx, std::vector<async_mqtt::buffer> &messages, std::string &topic)
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
        co_await amep_->send(
            async_mqtt::v5::connect_packet{
                true,
                0x1234,
                async_mqtt::allocate_buffer("cid2"),
                async_mqtt::nullopt,
                async_mqtt::nullopt,
                async_mqtt::nullopt,
            },
            asio::use_awaitable);
        co_await amep_->recv(async_mqtt::filter::match, {async_mqtt::control_packet_type::connack},
                             asio::use_awaitable);
        co_await send_subscribe();
    }

    auto send_subscribe() -> asio::awaitable<void> {
        std::optional<async_mqtt::packet_id_t> packet_id = amep_->acquire_unique_packet_id();
        auto sub_packet =
                async_mqtt::v5::subscribe_packet{
                    packet_id.value(),
                    {{async_mqtt::allocate_buffer(topic_), async_mqtt::qos::at_most_once}}
                };
        co_await amep_->send(sub_packet, asio::use_awaitable);
        co_await amep_->recv(async_mqtt::filter::match, {async_mqtt::control_packet_type::suback}, asio::use_awaitable);
        co_await receive_publish_packets();
    }

    auto receive_publish_packets() -> asio::awaitable<void> {
        while (true) {
            auto p =
                    co_await amep_->recv(async_mqtt::filter::match, {async_mqtt::control_packet_type::publish},
                                         asio::use_awaitable);
            async_mqtt::v5::publish_packet const &p2 = p.template get<async_mqtt::v5::publish_packet>();
            for (auto &payload: p2.payload()) {
                messages_.push_back(payload);
            }
        }
    }

    asio::io_context &io_ctx_;
    std::vector<async_mqtt::buffer> &messages_;
    std::string &topic_;

    decltype(async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>::create(
        async_mqtt::protocol_version::v5)) amep_;

    asio::ip::tcp::resolver resolver_{io_ctx_};
};

auto make_ncmd_payload(std::string variable_name) -> async_mqtt::v5::publish_packet {
    org::eclipse::tahu::protobuf::Payload payload;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::system_clock;

    auto timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    payload.set_timestamp(timestamp);

    payload.set_seq(1);

    org::eclipse::tahu::protobuf::Payload_Metric *metric = payload.add_metrics();

    metric->set_name(variable_name);
    metric->set_timestamp(timestamp);
    metric->set_datatype(11);
    metric->set_boolean_value(true);

    std::string payload_string;
    payload.SerializeToString(&payload_string);

    std::string ncmd_topic = "spBv1.0/tfc_unconfigured_group_id/NCMD/tfc_unconfigured_node_id";

    auto pub_packet =
            async_mqtt::v5::publish_packet{
                0, async_mqtt::allocate_buffer(ncmd_topic),
                async_mqtt::allocate_buffer(payload_string), async_mqtt::qos::at_most_once
            };

    return pub_packet;
}

auto main(int argc, char *argv[]) -> int {
    tfc::base::init(argc, argv);

    /// NOTE: broker is running on port 1965 because that port is never used for anything

    "correct nbirth with no signals"_test = [&]() {
        asio::io_context io_ctx{};

        // start broker
        mqtt_broker broker{io_ctx};
        io_ctx.run_for(std::chrono::milliseconds{5});

        // start client
        std::string nbirth_topic = "spBv1.0/tfc_unconfigured_group_id/NBIRTH/tfc_unconfigured_node_id";
        std::vector<async_mqtt::buffer> messages;
        mqtt_client cli{io_ctx, messages, nbirth_topic};
        io_ctx.run_for(std::chrono::milliseconds{5});

        // start mock ipc client
        tfc::ipc_ruler::ipc_manager_client_mock ipc_client{io_ctx};
        io_ctx.run_for(std::chrono::milliseconds{5});

        // start mqtt bridge
        tfc::mqtt::run<tfc::mqtt::config::bridge_mock, tfc::mqtt::client_semi_normal,
                    tfc::ipc_ruler::ipc_manager_client_mock &>
                running{io_ctx, ipc_client};
        co_spawn(io_ctx, running.start(), asio::detached);
        io_ctx.run_for(std::chrono::milliseconds{50});

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
        mqtt_broker broker2{io_ctx};
        io_ctx.run_for(std::chrono::milliseconds{5});

        // start client
        std::string ndata_topic = "spBv1.0/tfc_unconfigured_group_id/NDATA/tfc_unconfigured_node_id";
        std::vector<async_mqtt::buffer> messages2;
        mqtt_client ndata_cli{io_ctx, messages2, ndata_topic};
        io_ctx.run_for(std::chrono::milliseconds{5});

        // start mock ipc client
        tfc::ipc_ruler::ipc_manager_client_mock ipc_client2{io_ctx};
        tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock &> sig_b{
            io_ctx, ipc_client2,
            "test"
        };
        tfc::ipc::signal<tfc::ipc::details::type_string, tfc::ipc_ruler::ipc_manager_client_mock &> sig_s{
            io_ctx, ipc_client2,
            "test"
        };
        io_ctx.run_for(std::chrono::milliseconds{5});

        // send values on signals before startup
        sig_b.send(true);
        io_ctx.run_for(std::chrono::milliseconds{5});
        sig_s.send("Initial");
        io_ctx.run_for(std::chrono::milliseconds{5});

        // start mqtt bridge
        tfc::mqtt::run<tfc::mqtt::config::bridge_mock, tfc::mqtt::client_semi_normal,
                    tfc::ipc_ruler::ipc_manager_client_mock &>
                running{io_ctx, ipc_client2};
        co_spawn(io_ctx, running.start(), asio::detached);
        io_ctx.run_for(std::chrono::milliseconds{50});

        // send more values
        sig_b.send(false);
        io_ctx.run_for(std::chrono::milliseconds{5});
        sig_s.send("number_2");
        io_ctx.run_for(std::chrono::milliseconds{5});
        sig_b.send(true);
        io_ctx.run_for(std::chrono::milliseconds{5});
        sig_s.send("number_3");
        io_ctx.run_for(std::chrono::milliseconds{5});

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

    "dynamic loading of signals"_test = [&]() {
        asio::io_context io_ctx{};

        // start broker
        mqtt_broker broker2{io_ctx};
        io_ctx.run_for(std::chrono::milliseconds{5});

        // start client
        std::string nbirth_topic = "spBv1.0/tfc_unconfigured_group_id/NBIRTH/tfc_unconfigured_node_id";
        std::vector<async_mqtt::buffer> messages;
        mqtt_client cli{io_ctx, messages, nbirth_topic};
        io_ctx.run_for(std::chrono::milliseconds{5});

        // start mock ipc client
        tfc::ipc_ruler::ipc_manager_client_mock ipc_client{io_ctx};
        io_ctx.run_for(std::chrono::milliseconds{5});

        // start mqtt bridge
        tfc::mqtt::run<tfc::mqtt::config::bridge_mock, tfc::mqtt::client_semi_normal,
                    tfc::ipc_ruler::ipc_manager_client_mock &>
                running{io_ctx, ipc_client};
        co_spawn(io_ctx, running.start(), asio::detached);
        io_ctx.run_for(std::chrono::milliseconds{50});

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

        io_ctx.run_for(std::chrono::seconds{1});

        tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock &> sig_b{
            io_ctx, ipc_client,
            "test"
        };

        io_ctx.run_for(std::chrono::seconds{5});

        expect(messages.size() == 2);

        //  timestamp: 1706985768834
        // metrics {
        //     name: "Node Control/Rebirth"
        //     timestamp: 1706985768834
        //     datatype: 11
        //     is_historical: false
        //     is_transient: false
        //     is_null: false
        //     boolean_value: false
        //   }
        //  metrics {
        //      name: "bdSeq"
        //      timestamp: 1706985768834
        //      datatype: 8
        //      long_value: 0
        //    }
        //  metrics {
        //      name: "mqtt_bridge_integration_tests/def/bool/test"
        //      timestamp: 1706985768834
        //      datatype: 11
        //      is_historical: false
        //      is_transient: false
        //      is_null: true
        //      metadata {
        //          description: ""
        //        }
        //  }
        //  seq: 0

        org::eclipse::tahu::protobuf::Payload second_message;
        second_message.ParseFromArray(messages[1].data(), messages[1].size());
        expect(second_message.metrics_size() == 3);
        expect(second_message.has_seq());
        expect(second_message.seq() == 0);

        // rebirth metric
        expect(second_message.metrics()[0].name() == "Node Control/Rebirth");
        expect(second_message.metrics()[0].datatype() == 11);
        expect(!second_message.metrics()[0].is_historical());
        expect(!second_message.metrics()[0].is_transient());
        expect(!second_message.metrics()[0].is_null());
        expect(second_message.metrics()[0].has_boolean_value());
        expect(!second_message.metrics()[0].boolean_value());

        // bdSeq metric
        expect(second_message.metrics()[1].name() == "bdSeq");
        expect(second_message.metrics()[1].datatype() == 8);
        expect(second_message.metrics()[1].has_long_value());
        expect(second_message.metrics()[1].long_value() == 0);

        // new signal
        expect(second_message.metrics()[2].name() == "mqtt_bridge_integration_tests/def/bool/test");
        expect(second_message.metrics()[2].datatype() == 11);
        expect(!second_message.metrics()[2].is_historical());
        expect(!second_message.metrics()[2].is_transient());
        expect(second_message.metrics()[2].is_null());
    };

    "banned signals are not published"_test = [&]() {
        asio::io_context io_ctx{};

        // start broker
        mqtt_broker broker{io_ctx};
        io_ctx.run_for(std::chrono::milliseconds{5});

        // start client
        std::string ndata_topic = "spBv1.0/tfc_unconfigured_group_id/NBIRTH/tfc_unconfigured_node_id";
        std::vector<async_mqtt::buffer> messages;
        mqtt_client ndata_cli{io_ctx, messages, ndata_topic};
        io_ctx.run_for(std::chrono::milliseconds{5});

        // start mock ipc client
        tfc::ipc_ruler::ipc_manager_client_mock ipc_client{io_ctx};
        tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock &> sig_b{
            io_ctx, ipc_client,
            "test"
        };
        tfc::ipc::signal<tfc::ipc::details::type_string, tfc::ipc_ruler::ipc_manager_client_mock &> sig_s{
            io_ctx, ipc_client,
            "test"
        };
        io_ctx.run_for(std::chrono::milliseconds{5});

        // start mqtt bridge
        tfc::mqtt::run<tfc::mqtt::config::bridge_mock, tfc::mqtt::client_semi_normal,
                    tfc::ipc_ruler::ipc_manager_client_mock &>
                running{io_ctx, ipc_client};

        running.config().add_banned_signal("mqtt_bridge_integration_tests.def.bool.test");

        co_spawn(io_ctx, running.start(), asio::detached);
        io_ctx.run_for(std::chrono::milliseconds{50});

        expect(messages.size() == 1);

        org::eclipse::tahu::protobuf::Payload nbirth_message;
        nbirth_message.ParseFromArray(messages[0].data(), messages[0].size());
        expect(nbirth_message.metrics_size() == 3);
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

        // new signal
        expect(nbirth_message.metrics()[2].name() == "mqtt_bridge_integration_tests/def/string/test");
        expect(nbirth_message.metrics()[2].datatype() == 12);
        expect(!nbirth_message.metrics()[2].is_historical());
        expect(!nbirth_message.metrics()[2].is_transient());
        expect(nbirth_message.metrics()[2].is_null());
    };

    "writeable signal basic test"_test = [&]() {
        asio::io_context io_ctx{};

        // start broker
        mqtt_broker broker{io_ctx};
        io_ctx.run_for(std::chrono::milliseconds{5});

        // start client
        std::string topic = "";
        std::vector<async_mqtt::buffer> messages;
        mqtt_client client{io_ctx, messages, topic};
        io_ctx.run_for(std::chrono::milliseconds{5});

        // start mock ipc client
        tfc::ipc_ruler::ipc_manager_client_mock ipc_client{io_ctx};
        io_ctx.run_for(std::chrono::milliseconds{5});

        // start mqtt bridge
        tfc::mqtt::run<tfc::mqtt::config::bridge_mock, tfc::mqtt::client_semi_normal,
                    tfc::ipc_ruler::ipc_manager_client_mock &>
                running{io_ctx, ipc_client};

        running.config().add_writeable_signal("first", "first", tfc::ipc::details::type_e::_bool);

        co_spawn(io_ctx, running.start(), asio::detached);
        io_ctx.run_for(std::chrono::milliseconds{100});

        // should be one signal
        expect(ipc_client.signals_.size() == 1);
        expect(ipc_client.signals_[0].name == "mqtt_bridge_integration_tests.def.bool.first");

        tfc::ipc::slot<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock &> slot{
            io_ctx, ipc_client, "first", "first", [](bool) {
            }
        };
        io_ctx.run_for(std::chrono::milliseconds{1});

        ipc_client.connect("mqtt_bridge_integration_tests.def.bool.first",
                           "mqtt_bridge_integration_tests.def.bool.first",
                           [](std::error_code) {
                           });
        io_ctx.run_for(std::chrono::milliseconds{1});

        org::eclipse::tahu::protobuf::Payload payload;
        using std::chrono::duration_cast;
        using std::chrono::milliseconds;
        using std::chrono::system_clock;

        auto timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

        payload.set_timestamp(timestamp);

        payload.set_seq(1);

        org::eclipse::tahu::protobuf::Payload_Metric *metric = payload.add_metrics();

        metric->set_name("mqtt_bridge_integration_tests/def/bool/first");
        metric->set_timestamp(timestamp);
        metric->set_datatype(11);
        metric->set_boolean_value(true);

        std::string payload_string;
        payload.SerializeToString(&payload_string);

        std::string ncmd_topic = "spBv1.0/tfc_unconfigured_group_id/NCMD/tfc_unconfigured_node_id";

        auto pub_packet = async_mqtt::v5::publish_packet{
            0, async_mqtt::allocate_buffer(ncmd_topic),
            async_mqtt::allocate_buffer(payload_string), async_mqtt::qos::at_most_once
        };

        co_spawn(io_ctx,
                 client.amep_->send(pub_packet, asio::use_awaitable), asio::detached);

        io_ctx.run_for(milliseconds{10});

        std::cout << ipc_client.slots_[0].name << std::endl;
        std::cout << ipc_client.signals_[0].name << std::endl;

        expect(slot.value().has_value()) << slot.value().has_value();
        expect(slot.value().value() == true);
    };

    "writeable signal 2 signals"_test = [&]() {
        asio::io_context io_ctx{};

        // start broker
        mqtt_broker broker{io_ctx};
        io_ctx.run_for(std::chrono::milliseconds{5});

        // start client
        std::string topic = "";
        std::vector<async_mqtt::buffer> messages;
        mqtt_client client{io_ctx, messages, topic};
        io_ctx.run_for(std::chrono::milliseconds{5});

        // start mock ipc client
        tfc::ipc_ruler::ipc_manager_client_mock ipc_client{io_ctx};
        io_ctx.run_for(std::chrono::milliseconds{5});

        // start mqtt bridge
        tfc::mqtt::run<tfc::mqtt::config::bridge_mock, tfc::mqtt::client_semi_normal,
                    tfc::ipc_ruler::ipc_manager_client_mock &>
                running{io_ctx, ipc_client};

        running.config().add_writeable_signal("first", "first", tfc::ipc::details::type_e::_bool);
        running.config().add_writeable_signal("second", "second", tfc::ipc::details::type_e::_bool);

        co_spawn(io_ctx, running.start(), asio::detached);
        io_ctx.run_for(std::chrono::milliseconds{100});

        // should be one signal
        expect(ipc_client.signals_.size() == 2);
        expect(ipc_client.signals_[0].name == "mqtt_bridge_integration_tests.def.bool.first");
        expect(ipc_client.signals_[1].name == "mqtt_bridge_integration_tests.def.bool.second");

        tfc::ipc::slot<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock &> first_slot{
            io_ctx, ipc_client,
            "first", "first",
            [](bool) {
            }
        };
        tfc::ipc::slot<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock &> second_slot{
            io_ctx, ipc_client,
            "second", "second",
            [](bool) {
            }
        };
        io_ctx.run_for(std::chrono::milliseconds{1});

        ipc_client.connect("mqtt_bridge_integration_tests.def.bool.first",
                           "mqtt_bridge_integration_tests.def.bool.first",
                           [](std::error_code) {
                           });
        ipc_client.connect("mqtt_bridge_integration_tests.def.bool.second",
                           "mqtt_bridge_integration_tests.def.bool.second",
                           [](std::error_code) {
                           });
        io_ctx.run_for(std::chrono::milliseconds{1});

        co_spawn(io_ctx,
                 client.amep_->send(make_ncmd_payload("mqtt_bridge_integration_tests/def/bool/first"),
                                    asio::use_awaitable),
                 asio::detached);

        co_spawn(io_ctx,
                 client.amep_->send(make_ncmd_payload("mqtt_bridge_integration_tests/def/bool/second"),
                                    asio::use_awaitable),
                 asio::detached);
        io_ctx.run_for(std::chrono::milliseconds{100});

        expect(first_slot.value().has_value()) << first_slot.value().has_value();
        expect(first_slot.value().value() == true);

        expect(second_slot.value().has_value()) << second_slot.value().has_value();
        expect(second_slot.value().value() == true);
    };
  
    return 0;
}
#else
auto main() -> int {
  return 0;
}
#endif
