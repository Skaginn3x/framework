#pragma once

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>

// File under /etc/tfc/mqtt-broadcaster/def/mqtt_broadcaster.json which specifies the connection values for the MQTT broker.
struct config {
  std::string address{};
  std::string port{};
  std::string username{};
  std::string password{};
  std::string node_id{};
  std::string group_id{};
  std::string client_id{};
  std::array<std::tuple<std::string, std::string>, 10> scada_signals{};

  struct glaze {
    // clang-format off
    static constexpr auto value{ glz::object(
        "address", &config::address, "Hostname or IP address of the MQTT broker",
        "port", &config::port, "Port or service name of the MQTT broker",
        "username", &config::username, "Username for the MQTT broker",
        "password", &config::password, "Password for the MQTT broker",
        "node_id", &config::node_id, "Spark Plug B Node ID, used to identify which node is sending information",
        "group_id", &config::group_id, "Spark Plug B Group ID, used to identify which group the node belongs to",
        "client_id", &config::client_id, "Spark Plug B Client ID, used to identify which client is sending information",
        "scada_signals", &config::scada_signals, "Array of signals that SCADA can write to, each signal is a tuple of (signal name, signal type). Allowed types are: _bool, _double_t, _int64_t, _json, _string, _uint64_t"
      ) };
    // clang-format on
    static constexpr auto name{ "mqtt_broadcaster" };
  };
};
