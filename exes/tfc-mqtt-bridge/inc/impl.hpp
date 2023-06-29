#include "mqtt/async_client.h"
#include "paho2.hpp"

template <class ipc_client_type, class match_client>
class mqtt_broadcaster; // forward declaration

class MyActionListener : public mqtt::iaction_listener {
  void on_failure([[maybe_unused]] const mqtt::token& tok) override { std::cout << "Connection failed\n"; }

  void on_success([[maybe_unused]] const mqtt::token& tok) override { std::cout << "Connection succeeded\n"; }
};

template <class ipc_client_type, class match_client>
class MyCallback : public mqtt::callback {
public:
  MyCallback(mqtt_broadcaster<ipc_client_type, match_client>& owner)
      : owner_(owner) {}

  mqtt_broadcaster<ipc_client_type, match_client>& owner_;
  MyActionListener subListener_{};

  void connected(const std::string& cause) override { owner_.connected(cause); }

  void connection_lost(const std::string& cause) override { owner_.connection_lost(cause); }

  void message_arrived(mqtt::message::const_ptr_t msg) override { owner_.message_arrived(msg); }

  void delivery_complete(mqtt::delivery_token::ptr_t ptr) override { owner_.delivery_complete(ptr); }
};

