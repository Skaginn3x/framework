#pragma once

#include <tfc/confman.hpp>

namespace tfc::mqtt {

struct signal_definition {
  std::string name{};
  std::string description{};
  tfc::ipc::details::type_e type{ tfc::ipc::details::type_e::unknown };
  struct glaze {
    static constexpr std::string_view name{ "tfc::mqtt::signal_definition" };
    // clang-format off
        static constexpr auto value{ glz::object(
          "name", &signal_definition::name,
          "description", &signal_definition::description,
          "type", &signal_definition::type
        )};
    // clang-format on
  };
};

enum struct port_e : std::uint16_t { mqtt = 1883, mqtts = 8883 };

// File under /etc/tfc/mqtt-broadcaster/def/mqtt_broadcaster.json which specifies the connection values for the MQTT broker.
struct config {
  std::string address{};
  std::variant<port_e, uint16_t> port{};
  std::string username{};
  std::string password{};
  std::string node_id{};
  std::string group_id{};
  std::string client_id{};
  std::vector<signal_definition> scada_signals{};

  struct glaze {
    // clang-format off
    static constexpr auto value{ glz::object(
        "address", &config::address, "Hostname or IP address of the MQTT broker",
        "port", &config::port, "Port of the MQTT broker. Possible values are: mqtt, mqtts or a custom port number",
        "username", &config::username, "Username for the MQTT broker",
        "password", &config::password, "Password for the MQTT broker",
        "node_id", &config::node_id, "Spark Plug B Node ID, used to identify which node is sending information",
        "group_id", &config::group_id, "Spark Plug B Group ID, used to identify which group the node belongs to",
        "client_id", &config::client_id, "Spark Plug B Client ID, used to identify which client is sending information",
        "scada_signals", &config::scada_signals, "Array of signals that SCADA can write to"
      ) };
    // clang-format on
    static constexpr std::string_view name{ "mqtt_broadcaster" };
  };
};
}  // namespace tfc::mqtt

template <>
struct glz::meta<tfc::mqtt::port_e> {
  using enum tfc::mqtt::port_e;
  static constexpr std::string_view name{ "tfc::mqtt::port_e" };
  static constexpr auto value = glz::enumerate("mqtt", mqtt, "mqtts", mqtts);
};
