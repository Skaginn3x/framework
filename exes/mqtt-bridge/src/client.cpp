#include "client.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>

#include <async_mqtt/buffer.hpp>
#include <async_mqtt/packet/property.hpp>
#include <async_mqtt/packet/qos.hpp>
#include <async_mqtt/packet/v5_connect.hpp>
#include <async_mqtt/packet/v5_publish.hpp>
#include <async_mqtt/packet/will.hpp>

#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>

#include "endpoint.hpp"
#include "endpoint_mock.hpp"

namespace tfc::mqtt {
    template<class client_t, class config_t>
    client<client_t, config_t>::client(asio::io_context &io_ctx,
                                       std::string_view mqtt_will_topic,
                                       std::string_view mqtt_will_payload)
        : io_ctx_(io_ctx), mqtt_will_topic_(mqtt_will_topic), mqtt_will_payload_(mqtt_will_payload) {
        using enum structs::ssl_active_e;
        if (config_.value().ssl_active == yes) {
            endpoint_client_ = std::make_unique<client_t>(io_ctx_, yes);
        } else if (config_.value().ssl_active == no) {
            endpoint_client_ = std::make_unique<client_t>(io_ctx_, no);
        }
    }

    template<class client_t, class config_t>
    auto client<client_t, config_t>::connect() -> asio::awaitable<bool> {
        logger_.trace("Resolving the MQTT broker address...");

        asio::ip::tcp::socket resolve_sock{io_ctx_};
        asio::ip::tcp::resolver res{resolve_sock.get_executor()};

        asio::ip::tcp::resolver::results_type resolved_ip =
                co_await res.async_resolve(config_.value().address, config_.value().get_port(), asio::use_awaitable);

        auto connection_successful = co_await connect_and_handshake(resolved_ip);

        if (connection_successful) {
            co_return co_await send_initial();
        }
        co_return false;
    }

    template<class client_t, class config_t>
    auto client<client_t, config_t>::connect_and_handshake(asio::ip::tcp::resolver::results_type resolved_ip)
        -> asio::awaitable<bool> {
        logger_.trace("Resolved the MQTT broker address. Connecting...");

        logger_.trace("Async connect for mqtt client");
        co_await endpoint_client_->async_connect(resolved_ip);

        logger_.trace("Setting SSL SNI");

        endpoint_client_->set_sni_hostname(config_.value().address);

        logger_.trace("Starting SSL handshake");

        co_await endpoint_client_->async_handshake();

        /// SparkPlugB spec specifies that clean start must be true and Session Expiry Interval must be 0
        auto connect_packet =
                async_mqtt::v5::connect_packet{
                    true,
                    std::chrono::seconds(100).count(),
                    async_mqtt::allocate_buffer(config_.value().client_id),
                    async_mqtt::will(async_mqtt::allocate_buffer(mqtt_will_topic_),
                                     async_mqtt::buffer(std::string_view{mqtt_will_payload_}),
                                     {async_mqtt::qos::at_least_once | async_mqtt::pub::retain::no}),
                    async_mqtt::allocate_buffer(config_.value().username),
                    async_mqtt::allocate_buffer(config_.value().password),
                    {async_mqtt::property::session_expiry_interval{0}}
                };

        logger_.trace("Sending MQTT connection packet...");

        auto send_error = co_await endpoint_client_->send(connect_packet, asio::use_awaitable);

        if (send_error) {
            co_return false;
        }

        logger_.trace("MQTT connection packet sent. Waiting for CONNACK...");

        auto connack = co_await receive_connack();
        logger_.trace("CONNACK received: {}", connack);

        co_return connack;
    }

    template<class client_t, class config_t>
    auto client<client_t, config_t>::receive_connack() -> asio::awaitable<bool> {
        logger_.trace("Waiting for CONNACK");

        auto connack_received = co_await endpoint_client_->recv(async_mqtt::control_packet_type::connack);

        logger_.trace("CONNACK received");
        auto connack_packet = connack_received.template get<async_mqtt::v5::connack_packet>();

        logger_.trace("Checking if CONNACK is valid or not");
        co_return connack_packet.code() == async_mqtt::connect_reason_code::success;
    }

    template<class client_t, class config_t>
    auto client<client_t, config_t>::send_message(std::string topic, std::string payload, async_mqtt::qos qos)
        -> asio::awaitable<bool> {
        std::optional<uint16_t> p_id;

        if (qos != async_mqtt::qos::at_most_once) {
            p_id = endpoint_client_->acquire_unique_packet_id();
        } else {
            p_id = 0;
        }

        auto pub_packet = async_mqtt::v5::publish_packet{
            p_id.value(), async_mqtt::allocate_buffer(topic),
            async_mqtt::allocate_buffer(payload), qos
        };

        auto send_error = co_await endpoint_client_->send(pub_packet, asio::use_awaitable);

        if (send_error) {
            co_return false;
        }
        co_return true;
    }

    template<class client_t, class config_t>
    auto client<client_t, config_t>::subscribe_to_topic(std::string topic) -> asio::awaitable<bool> {
        logger_.trace("Subscribing to topic: {}", topic);
        logger_.trace("Sending subscription packet...");

        std::optional<uint16_t> p_id;

        p_id = endpoint_client_->acquire_unique_packet_id();

        std::string_view topic_view{topic};

        auto sub_packet = async_mqtt::v5::subscribe_packet{
            p_id.value(), {{async_mqtt::buffer(topic_view), async_mqtt::qos::at_most_once | async_mqtt::sub::nl::yes}}
        };

        auto send_error = co_await endpoint_client_->send(sub_packet, asio::use_awaitable);

        if (send_error) {
            co_return !send_error;
        }

        logger_.trace("Subscription packet sent. Waiting for SUBACK...");

        auto suback_received = co_await endpoint_client_->recv(async_mqtt::control_packet_type::suback);
        logger_.trace("SUBACK received. Checking for errors...");

        auto suback_packet = suback_received.template get<async_mqtt::v5::suback_packet>();

        auto *suback_packet_ptr = suback_received.template get_if<async_mqtt::v5::suback_packet>();

        if (suback_packet_ptr == nullptr) {
            logger_.error("Received packet is not a SUBACK packet");
            co_return false;
        }

        for (auto const &entry: suback_packet.entries()) {
            if (entry != async_mqtt::suback_reason_code::granted_qos_0) {
                logger_.error("Error subscribing to topic: {}, reason code: {}", topic.data(),
                              async_mqtt::suback_reason_code_to_str(entry));
                co_return false;
            }
        }

        co_return true;
    }

    template<class client_t, class config_t>
    auto client<client_t, config_t>::wait_for_payloads(
        std::function<void(async_mqtt::buffer const &data, async_mqtt::v5::publish_packet publish_packet)>
        process_payload,
        bool &restart_needed) -> asio::awaitable<void> {
        while (true) {
            logger_.trace("Waiting for Publish packets");

            auto publish_recv = co_await endpoint_client_->recv(async_mqtt::control_packet_type::publish);

            logger_.trace("Publish packet received, parsing...");

            auto *publish_packet = publish_recv.template get_if<async_mqtt::v5::publish_packet>();

            if (publish_packet == nullptr) {
                logger_.error("Received packet is not a PUBLISH packet");
                restart_needed = true;
                co_return;
            }

            logger_.trace("Received PUBLISH packet. Parsing payload...");

            for (uint64_t i = 0; i < publish_packet->payload().size(); i++) {
                asio::co_spawn(strand(),
                               process_payload(publish_packet->payload()[i], *publish_packet), asio::detached);
            }
        }
    }

    template<class client_t, class config_t>
    auto client<client_t, config_t>::strand() -> asio::strand<asio::any_io_executor> {
        return endpoint_client_->strand();
    }

    template<class client_t, class config_t>
    auto client<client_t, config_t>::set_initial_message(std::string const &topic,
                                                         std::string const &payload,
                                                         async_mqtt::qos const &qos) -> void {
        initial_message_ = std::tuple<std::string, std::string, async_mqtt::qos>{topic, payload, qos};
    }

    template<class client_t, class config_t>
    auto client<client_t, config_t>::send_initial() -> asio::awaitable<bool> {
        if (!std::get<0>(initial_message_).empty()) {
            co_return co_await send_message(std::get<0>(initial_message_), std::get<1>(initial_message_),
                                            std::get<2>(initial_message_));
        }
        co_return true;
    }
} // namespace tfc::mqtt

template class tfc::mqtt::client<tfc::mqtt::endpoint_client, tfc::confman::config<tfc::mqtt::config::broker> >;

template class tfc::mqtt::client<tfc::mqtt::endpoint_client_mock, tfc::mqtt::config::broker_mock>;
