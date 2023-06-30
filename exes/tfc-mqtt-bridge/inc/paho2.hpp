#pragma once
#include <tfc/confman/observable.hpp>
#include <tfc/ipc.hpp>
#include "impl.hpp"

struct config {
  tfc::confman::observable<std::vector<std::string>> _banned_topics{};
  struct glaze {
    static constexpr auto value{ glz::object("_banned_topics", &config::_banned_topics) };
    static constexpr auto name{ "mqtt_broadcaster" };
  };
};

template <class ipc_client_type, class match_client>
class mqtt_broadcaster {
public:
  explicit mqtt_broadcaster(std::shared_ptr<mqtt::async_client> mqtt_client)
      : mqtt_client_{ std::move(mqtt_client) }, listener_{ *this }, callbacks_{ *this } {
    mqtt::connect_options connection_options;
    connection_options.set_clean_session(false);

    mqtt_client_->set_callback(callbacks_);

    try {
      std::cout << "Connecting to the MQTT server..." << std::flush;
      mqtt_client_->connect(connection_options, nullptr, listener_);
    } catch (const mqtt::exception& exc) {
      std::cerr << "Unable to connect, exception: " << exc.what() << "\n";
    }
  }

  void connected(const std::string& cause) {
    std::cout << "Connection success\n";
    std::cout << "cause: " << cause << "\n";
    mqtt_client_->subscribe(topic_, qos_, nullptr, listener_);
  }

  void connection_lost(const std::string& cause) {
    std::cout << "Connection lost\n";
    if (!cause.empty()) {
      std::cout << "cause: " << cause << "\n";
    }
  }

  void message_arrived(mqtt::message::const_ptr_t& msg) {
    std::cout << "Message arrived\n";
    std::cout << "topic: '" << msg->get_topic() << "'\n";
    std::cout << "payload: '" << msg->to_string() << "'\n";

    // replace all / with .
    std::string topic{ msg->get_topic() };
    std::replace(topic.begin(), topic.end(), '/', '.');

    std::cout << "signal: '" << topic << "'\n";
  }

  void delivery_complete(mqtt::delivery_token::ptr_t& ptr) {
    std::cout << "Delivery complete for token: " << (ptr ? ptr->get_message_id() : -1) << "\n";
  }

private:
  std::shared_ptr<mqtt::async_client> mqtt_client_;
  action_listener<ipc_client_type, match_client> listener_;
  callback_listener<ipc_client_type, match_client> callbacks_;
  const int qos_ = 1;
  const std::string topic_ = "#";
};
