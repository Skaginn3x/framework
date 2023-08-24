#include <boost/program_options.hpp>
#include <boost/ut.hpp>
#include <tfc/ipc.hpp>

#include "mock_client.hpp"
#include "mqtt_broadcaster.hpp"

namespace ut = boost::ut;
using boost::ut::operator""_test;
using org::eclipse::tahu::protobuf::Payload;

namespace tfc {

class testing_mqtt_broadcaster {
public:
  explicit testing_mqtt_broadcaster(tfc::mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client_mock,
                                                          mock_mqtt_client<async_mqtt::role, async_mqtt::protocol::mqtts>,
                                                          mock_config,
                                                          network_manager_mock>& application)
      : application_(application) {}

  auto verify_nbirth_message() -> void {
    auto packet = application_.mqtt_client_->get_last_message();

    ut::expect(std::holds_alternative<async_mqtt::v5::publish_packet>(packet));

    auto* pub_packet = std::get_if<async_mqtt::v5::publish_packet>(&packet);

    ut::expect(pub_packet->topic() == "spBv1.0/group_id/NBIRTH/edge_node_id");

    // [tck-id-payloads-nbirth-qos]
    ut::expect(pub_packet->opts().get_qos() == async_mqtt::qos::at_most_once);

    // [tck-id-payloads-nbirth-retain]
    ut::expect(pub_packet->opts().get_retain() == async_mqtt::pub::retain::no);

    Payload payload;

    const bool payload_valid = payload.ParseFromArray(pub_packet->payload()[0].data(), pub_packet->payload()[0].size());

    if (!payload_valid) {
      ut::expect(false);
    }

    // [tck-id-payloads-nbirth-seq]
    ut::expect(payload.has_seq());
    ut::expect(payload.seq() == 0);

    // [tck-id-payloads-nbirth-timestamp]
    ut::expect(payload.has_timestamp());
    ut::expect(payload.timestamp() != 0);

    auto metrics = payload.metrics();

    ut::expect(metrics.size() == 1);

    // [tck-id-payloads-nbirth-rebirth-req]
    ut::expect(metrics[0].name() == "Node Control/Rebirth");
    ut::expect(metrics[0].datatype() == 11);
    ut::expect(!metrics[0].boolean_value());
  }

  auto verify_ndata_message() -> void {
    auto packet = application_.mqtt_client_->get_last_message();

    ut::expect(std::holds_alternative<async_mqtt::v5::publish_packet>(packet));

    auto* pub_packet = std::get_if<async_mqtt::v5::publish_packet>(&packet);

    ut::expect(pub_packet->topic() == "spBv1.0/group_id/NDATA/edge_node_id");

    Payload payload;

    const bool payload_valid = payload.ParseFromArray(pub_packet->payload()[0].data(), pub_packet->payload()[0].size());

    if (!payload_valid) {
      ut::expect(false);
    }

    auto metrics = payload.metrics();

    // [tck-id-payloads-ndata-timestamp]
    ut::expect(payload.has_timestamp());

    // [tck-id-payloads-ndata-seq]
    ut::expect(payload.has_seq());

    // [tck-id-payloads-ndata-seq-inc]
    ut::expect(payload.seq() < 256);

    // [tck-id-payloads-ndata-qos]
    ut::expect(pub_packet->opts().get_qos() == async_mqtt::qos::at_most_once);

    // [tck-id-payloads-ndata-retain]
    ut::expect(pub_packet->opts().get_retain() == async_mqtt::pub::retain::no);
  }

  auto test_timestamp_milliseconds() -> void {
    const std::chrono::milliseconds first_time = application_.timestamp_milliseconds();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    const std::chrono::milliseconds second_time = application_.timestamp_milliseconds();
    ut::expect(first_time < second_time);

    const std::chrono::milliseconds current_time = application_.timestamp_milliseconds();
    const std::chrono::milliseconds t_now =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    ut::expect(current_time > std::chrono::milliseconds(0));
    ut::expect(current_time <= t_now);
  }

  auto test_get_payload() -> void {
    const Payload payload1 = application_.make_payload();

    ut::expect(payload1.timestamp() > static_cast<uint64_t>(std::chrono::milliseconds(0).count()));
    ut::expect(payload1.seq() < 256);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto payload2 = application_.make_payload();

    ut::expect(payload2.timestamp() > payload1.timestamp());

    ut::expect(payload2.seq() == (payload1.seq() + 1) % 256);

    application_.seq_ = 255;
    auto payload3 = application_.make_payload();
    ut::expect(payload3.seq() == 0);
  }

  auto test_type_enum_convert() -> void {
    ut::expect(application_.type_enum_convert(tfc::ipc::details::type_e::unknown) == 0);
    ut::expect(application_.type_enum_convert(tfc::ipc::details::type_e::_bool) == 11);
    ut::expect(application_.type_enum_convert(tfc::ipc::details::type_e::_int64_t) == 4);
    ut::expect(application_.type_enum_convert(tfc::ipc::details::type_e::_uint64_t) == 8);
    ut::expect(application_.type_enum_convert(tfc::ipc::details::type_e::_double_t) == 10);
    ut::expect(application_.type_enum_convert(tfc::ipc::details::type_e::_string) == 12);
    ut::expect(application_.type_enum_convert(tfc::ipc::details::type_e::_json) == 12);
    ut::expect(application_.type_enum_convert(static_cast<tfc::ipc::details::type_e>(999)) == 0);
  }

  auto test_format_signal_name() -> void {
    const std::string str1 = "noDotInHere";
    ut::expect(application_.format_signal_name(str1) == str1);

    const std::string str2 = "dot.here";
    ut::expect(application_.format_signal_name(str2) == "dot/here");

    const std::string str3 = "dot.here.and.here";
    ut::expect(application_.format_signal_name(str3) == "dot/here/and/here");

    const std::string str4 = ".dotAtEnds.";
    ut::expect(application_.format_signal_name(str4) == "/dotAtEnds/");

    const std::string str5;
    ut::expect(application_.format_signal_name(str5) == str5);
  }

  auto test_topic_formatter() -> void {
    const std::vector<std::string_view> empty_vector = {};
    try {
      const std::string result = application_.topic_formatter(empty_vector);
      ut::expect(false);
    } catch (const std::runtime_error& e) {
      ut::expect(std::string(e.what()) == "Topic can not be empty");
    }

    const std::vector<std::string_view> single_element_vector = { "singleElement" };
    ut::expect(application_.topic_formatter(single_element_vector) == "singleElement");

    const std::vector<std::string_view> multiple_element_vector = { "element1", "element2", "element3" };
    ut::expect(application_.topic_formatter(multiple_element_vector) == "element1/element2/element3");
  }

  auto test_set_value_payload_any() -> void {
    Payload_Metric metric;

    application_.set_value_payload(&metric, true);
    ut::expect(metric.boolean_value());

    application_.set_value_payload(&metric, std::string("test"));
    ut::expect(metric.string_value() == "test");

    application_.set_value_payload(&metric, static_cast<uint64_t>(10));
    ut::expect(metric.long_value() == 10);

    application_.set_value_payload(&metric, static_cast<int64_t>(-10));
    const uint64_t res = 18446744073709551606U;
    ut::expect(metric.long_value() == res);

    application_.set_value_payload(&metric, 3.14);
    double difference_d = std::abs(metric.double_value() - 3.14);
    ut::expect(difference_d < 0.0001);

    application_.set_value_payload(&metric, static_cast<float>(2.71));
    float difference_f = std::abs(metric.float_value() - 2.71F);
    ut::expect(difference_f < 0.0001F);

    application_.set_value_payload(&metric, static_cast<uint32_t>(20));
    ut::expect(metric.int_value() == 20);

    try {
      application_.set_value_payload(&metric, std::make_tuple(1, 2, 3));
      ut::expect(false);
    } catch (const std::runtime_error& e) {
      ut::expect(std::string(e.what()) == "Unexpected type in std::any.");
    }
  }

  auto test_set_value_payload() -> void {
    Payload_Metric metric;

    bool boolean_payload = true;
    application_.set_value_payload(&metric, boolean_payload);
    ut::expect(metric.boolean_value() == boolean_payload);

    std::string string_payload = "test";
    application_.set_value_payload(&metric, string_payload);
    ut::expect(metric.string_value() == string_payload);

    uint64_t ui64 = 1234567890;
    application_.set_value_payload(&metric, ui64);
    ut::expect(metric.long_value() == ui64);

    int64_t i64 = -1234567890;
    application_.set_value_payload(&metric, i64);
    ui64 = 18446744072474983726U;
    ut::expect(metric.long_value() == ui64);

    double double_payload = 1.234567890;
    application_.set_value_payload(&metric, double_payload);
    double difference_d = std::abs(metric.double_value() - double_payload);
    ut::expect(difference_d < 0.0001);

    float float_payload = 1.23F;
    application_.set_value_payload(&metric, float_payload);
    float difference_f = std::abs(metric.float_value() - float_payload);
    ut::expect(difference_f < 0.0001F);

    uint32_t ui32 = 123456;
    application_.set_value_payload(&metric, ui32);
    ut::expect(metric.int_value() == ui32);
  }

  auto test_connect_to_broker(asio::io_context& io_ctx) -> void {
    asio::co_spawn(application_.mqtt_client_->strand(), application_.connect_to_broker(), asio::detached);

    io_ctx.run_for(std::chrono::seconds(1));

    auto packet = application_.mqtt_client_->get_last_message();

    ut::expect(std::holds_alternative<async_mqtt::v5::connect_packet>(packet));

    auto* conn_packet = std::get_if<async_mqtt::v5::connect_packet>(&packet);

    // [tck-id-principles-persistence-clean-session-50]
    for (const auto& property : conn_packet->props()) {
      auto const& sei_prop = property.get<async_mqtt::property::session_expiry_interval>();
      ut::expect(sei_prop.val() == 0);
    }

    ut::expect(conn_packet->clean_start());

    // [tck-id-payloads-ndeath-will-message]
    ut::expect(conn_packet->get_will()->topic() == "spBv1.0/group_id/NDEATH/edge_node_id");

    // [tck-id-payloads-ndeath-will-message-retain]
    ut::expect(conn_packet->get_will()->get_retain() == async_mqtt::pub::retain::no);
    ut::expect(conn_packet->get_will()->message() == "DEATH");
    // [tck-id-payloads-ndeath-will-message-qos]
    ut::expect(conn_packet->get_will()->get_qos() == async_mqtt::qos::at_least_once);
  }

  auto test_process_ncmd_packet(asio::io_context& ctx) -> void {
    asio::co_spawn(application_.mqtt_client_->strand(), application_.process_ncmd_packet(), asio::detached);
    ctx.run_for(std::chrono::milliseconds(10));
    verify_nbirth_message();
  }

  auto test_subscribe_to_ncmd_topic(asio::io_context& ctx) -> void {
    asio::co_spawn(application_.mqtt_client_->strand(), application_.subscribe_to_ncmd_topic(), asio::detached);

    ctx.run_for(std::chrono::seconds(1));

    auto packet = application_.mqtt_client_->get_last_message();

    ut::expect(std::holds_alternative<async_mqtt::v5::subscribe_packet>(packet));

    auto* sub_packet = std::get_if<async_mqtt::v5::subscribe_packet>(&packet);

    // if the client sends a packet to the topic, it won't receive the same packet through its subscription
    ut::expect(sub_packet->entries()[0].opts().get_nl() == async_mqtt::sub::nl::yes);

    // [tck-id-payloads-ncmd-qos]
    ut::expect(sub_packet->entries()[0].opts().get_qos() == async_mqtt::qos::at_most_once);
    ut::expect(sub_packet->entries()[0].topic() == "spBv1.0/group_id/NCMD/edge_node_id");
  }

  auto test_receive_and_send_message(asio::io_context& ctx) -> void {
    const std::string signal_name = "test_signal_bool";

    tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock> send_signal(
        ctx, application_.ipc_client_, signal_name, signal_name);

    std::string full_signal_name = "mqtt-broadcaster-unit-tests.def.bool.test_signal_bool";

    auto ipc = tfc::ipc::details::create_ipc_recv<tfc::ipc::details::any_recv>(ctx, full_signal_name);

    std::visit(
        [&](auto&& receiver) -> void {
          using receiver_t = std::remove_cvref_t<decltype(receiver)>;
          if constexpr (!std::same_as<receiver_t, std::monostate>) {
            auto error_code = receiver->connect(full_signal_name);
            if (error_code) {
              ut::expect(false);
            }
          }
        },
        ipc);

    const tfc::ipc_ruler::signal test_signal{ full_signal_name,
                                              tfc::ipc_ruler::type_e::_bool,
                                              "test",
                                              std::chrono::system_clock::now(),
                                              std::chrono::system_clock::now(),
                                              "test" };

    signal_data send_signal_info{ test_signal, std::move(ipc), std::nullopt };

    asio::co_spawn(application_.mqtt_client_->strand(), application_.receive_and_send_message(send_signal_info),
                   asio::detached);

    send_signal.send(true);

    ctx.run_for(std::chrono::milliseconds(5));

    verify_ndata_message();
  }

  auto test_receive_and_send_message_with_load(asio::io_context& ctx) -> void {
    const std::string signal_name = "test_signal_bool";

    tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock> send_signal(
        ctx, application_.ipc_client_, signal_name, signal_name);

    std::string signal_value = "mqtt-broadcaster-unit-tests.def.bool.test_signal_bool";

    auto ipc = tfc::ipc::details::create_ipc_recv<tfc::ipc::details::any_recv>(ctx, signal_value);

    std::visit(
        [&](auto&& receiver) -> void {
          using receiver_t = std::remove_cvref_t<decltype(receiver)>;
          if constexpr (!std::same_as<receiver_t, std::monostate>) {
            auto error_code = receiver->connect(signal_value);
            if (error_code) {
              ut::expect(false);
            }
          }
        },
        ipc);

    const tfc::ipc_ruler::signal test_signal{ signal_value,
                                              tfc::ipc_ruler::type_e::_bool,
                                              "test",
                                              std::chrono::system_clock::now(),
                                              std::chrono::system_clock::now(),
                                              "test" };

    signal_data send_signal_info{ test_signal, std::move(ipc), std::nullopt };

    asio::co_spawn(application_.mqtt_client_->strand(), application_.receive_and_send_message(send_signal_info),
                   asio::detached);

    // flip true/false
    for (int i = 0; i < 300; i++) {
      send_signal.send(i % 2 == 0);
      ctx.run_for(std::chrono::milliseconds(10));
    }

    verify_ndata_message();
  }

  auto test_send_nbirth(asio::io_context& ctx) -> void {
    asio::co_spawn(application_.mqtt_client_->strand(), application_.send_nbirth(), asio::detached);
    ctx.run_for(std::chrono::milliseconds(10));
    verify_nbirth_message();
  }

  auto test_send_message(asio::io_context& ctx) -> void {
    const std::string topic = "test_topic";
    const std::string payload = "test_payload";

    asio::co_spawn(application_.mqtt_client_->strand(),
                   application_.send_message(topic, payload, async_mqtt::qos::at_most_once), asio::detached);

    ctx.run_for(std::chrono::milliseconds(200));

    auto packet = application_.mqtt_client_->get_last_message();

    ut::expect(std::holds_alternative<async_mqtt::v5::publish_packet>(packet));

    auto* pub_packet = std::get_if<async_mqtt::v5::publish_packet>(&packet);

    ut::expect(std::string(pub_packet->topic()) == topic) << std::string(pub_packet->topic()) << topic;
    ut::expect(std::string(pub_packet->payload()[0]) == payload) << pub_packet->payload()[0] << payload;

    ut::expect(pub_packet->opts().get_qos() == async_mqtt::qos::at_most_once)
        << pub_packet->opts().get_qos() << async_mqtt::qos::at_most_once;

    // SparkPlugB specifies that if qos is 0, packet_id must be 0
    ut::expect(pub_packet->packet_id() == 0);
  }

  int seq = 0;

private:
  tfc::mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client_mock,
                        mock_mqtt_client<async_mqtt::role, async_mqtt::protocol::mqtts>,
                        mock_config,
                        network_manager_mock>& application_;
};
}  // namespace tfc

auto normal_unit_tests() -> void {
  asio::io_context io_ctx{};
  tfc::mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client_mock,
                        mock_mqtt_client<async_mqtt::role, async_mqtt::protocol::mqtts>, mock_config, network_manager_mock>
      application(io_ctx);

  tfc::testing_mqtt_broadcaster test{ application };

  "Testing timestamp"_test = [&test] { test.test_timestamp_milliseconds(); };

  "Testing get_payload"_test = [&test] { test.test_get_payload(); };

  "Testing type_enum_convert"_test = [&test] { test.test_type_enum_convert(); };

  "Testing format_signal_name"_test = [&test] { test.test_format_signal_name(); };

  "Testing topic_formatter"_test = [&test] { test.test_topic_formatter(); };

  "Testing set_value_payload with std::any implementation"_test = [&test] { test.test_set_value_payload_any(); };

  "Testing set_value_payload"_test = [&test] { test.test_set_value_payload(); };
}

auto networking_tests() -> void {
  "Test connect to broker"_test = [] {
    asio::io_context io_ctx{};
    tfc::mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client_mock,
                          mock_mqtt_client<async_mqtt::role, async_mqtt::protocol::mqtts>, mock_config, network_manager_mock>
        application(io_ctx);

    tfc::testing_mqtt_broadcaster test{ application };

    test.test_connect_to_broker(io_ctx);
  };

  "Test subscribe to NCMD topic"_test = [] {
    asio::io_context io_ctx{};
    tfc::mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client_mock,
                          mock_mqtt_client<async_mqtt::role, async_mqtt::protocol::mqtts>, mock_config, network_manager_mock>
        application(io_ctx);

    tfc::testing_mqtt_broadcaster test{ application };

    test.test_subscribe_to_ncmd_topic(io_ctx);
  };

  "Test process NCMD topic"_test = [] {
    asio::io_context io_ctx{};
    tfc::mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client_mock,
                          mock_mqtt_client<async_mqtt::role, async_mqtt::protocol::mqtts>, mock_config, network_manager_mock>
        application(io_ctx);

    tfc::testing_mqtt_broadcaster test{ application };

    test.test_process_ncmd_packet(io_ctx);
  };

  "Test receive and send messages"_test = [] {
    asio::io_context io_ctx{};
    tfc::mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client_mock,
                          mock_mqtt_client<async_mqtt::role, async_mqtt::protocol::mqtts>, mock_config, network_manager_mock>
        application(io_ctx);

    tfc::testing_mqtt_broadcaster test{ application };

    test.test_receive_and_send_message(io_ctx);
  };

  "Test receive and send messages"_test = [] {
    asio::io_context io_ctx{};
    tfc::mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client_mock,
                          mock_mqtt_client<async_mqtt::role, async_mqtt::protocol::mqtts>, mock_config, network_manager_mock>
        application(io_ctx);

    tfc::testing_mqtt_broadcaster test{ application };

    test.test_receive_and_send_message_with_load(io_ctx);
  };

  "Test receive and send messages"_test = [] {
    asio::io_context io_ctx{};
    tfc::mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client_mock,
                          mock_mqtt_client<async_mqtt::role, async_mqtt::protocol::mqtts>, mock_config, network_manager_mock>
        application(io_ctx);

    tfc::testing_mqtt_broadcaster test{ application };

    test.test_send_nbirth(io_ctx);
  };

  "Test receive and send messages"_test = [] {
    asio::io_context io_ctx{};
    tfc::mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client_mock,
                          mock_mqtt_client<async_mqtt::role, async_mqtt::protocol::mqtts>, mock_config, network_manager_mock>
        application(io_ctx);

    tfc::testing_mqtt_broadcaster test{ application };
    test.test_send_message(io_ctx);
  };
}

auto main(int argc, char* argv[]) -> int {
  auto program_description{ tfc::base::default_description() };
  tfc::base::init(argc, argv, program_description);

  normal_unit_tests();
  networking_tests();

  return 0;
}
