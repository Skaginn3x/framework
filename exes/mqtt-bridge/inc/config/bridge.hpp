#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include <boost/asio.hpp>
#include <glaze/core/common.hpp>

#include <tfc/utils/json_schema.hpp>

#include <string>
#include <string_view>
#include <structs.hpp>
#include <vector>

#include <glaze/core/common.hpp>
#include <tfc/utils/json_schema.hpp>

#include <signal_names.hpp>

namespace tfc::mqtt::config {

namespace asio = boost::asio;
using ipc::details::type_e;

struct signal_definition {
  std::string name{};
  std::string description{};
  type_e type{ type_e::unknown };
  struct glaze {
    // clang-format off
    static constexpr auto value{glz::object(
            "name", &signal_definition::name,
            "description", &signal_definition::description,
            "type", &signal_definition::type
    )};
    // clang-format on
    static constexpr std::string_view name{ "tfc::mqtt::signal_definition" };
  };
};

struct signal_name {
  std::string value{};

  struct glaze {
    static constexpr auto value = &signal_name::value;
    static constexpr std::string_view name{ "signal_name" };
  };
};

enum struct port_e : uint16_t { mqtt = 1883, mqtts = 8883 };
}  // namespace tfc::mqtt::config

template <>
struct glz::meta<tfc::mqtt::config::port_e> {
  using enum tfc::mqtt::config::port_e;
  static constexpr std::string_view name{ "tfc::mqtt::port_e" };
  static constexpr auto value = enumerate("mqtt", mqtt, "mqtts", mqtts);
};

template <>
struct glz::meta<tfc::mqtt::structs::ssl_active_e> {
  using enum tfc::mqtt::structs::ssl_active_e;
  static constexpr std::string_view name{ "tfc::mqtt::ssl" };
  static constexpr auto value = enumerate("yes", yes, "no", no);
};

template <>
struct tfc::json::detail::to_json_schema<tfc::mqtt::config::signal_name> {
  template <auto Opts>
  static void op(auto& s, auto&) noexcept {
    if (!s.oneOf.has_value()) {
      s.oneOf = std::vector<schematic>{};
    }
    for (auto const& signal : global::get_signals()) {
      s.oneOf.value().push_back(schematic{
          .attributes{ schema{ .title = signal.name, .description = signal.description, .constant = signal.name } } });
    }
  }
};

namespace tfc::mqtt::config {
struct bridge {
  std::vector<signal_name> banned_signals{};
  std::string node_id{ "tfc_unconfigured_node_id" };
  std::string group_id{ "tfc_unconfigured_group_id" };
  std::string address{ "localhost" };
  std::variant<port_e, uint16_t> port{ port_e::mqtt };
  structs::ssl_active_e ssl_active{ structs::ssl_active_e::no };
  std::string username{};
  std::string password{};
  std::string client_id{ "tfc_unconfigured_client_id" };
  std::vector<signal_definition> writeable_signals{};

  struct glaze {
    static constexpr auto value{ glz::object(
        // clang-format off
       "node_id", &bridge::node_id, json::schema{ .description = "Spark Plug B Node ID, used to identify which node is sending information", .pattern = "[^+#/]" },
        "group_id", &bridge::group_id, json::schema{ .description = "Spark Plug B Group ID, used to identify which group the node belongs to", .pattern = "[^+#/]" },
        "banned_signals", &bridge::banned_signals, "Signals not to publish",
        "address", &bridge::address, "Hostname or IP address of the MQTT broker",
        "port", &bridge::port, "Port of the MQTT broker. Possible values are: mqtt, mqtts or a custom port number",
        "ssl_active", &bridge::ssl_active, "Whether or not to use SSL to connect to the MQTT broker",
        "username", &bridge::username, "Username for the MQTT broker",
        "password", &bridge::password, "Password for the MQTT broker",
        "client_id", &bridge::client_id, "Client ID, used to identify which client is sending information",
        "writeable_signals", &bridge::writeable_signals, "Array of signals that an MQTT client with a Spark Plug B extension can write to"

        ) };
    // clang-format on
  };
  static constexpr std::string_view name{ "mqtt_config" };

  [[nodiscard]] auto get_port() const -> std::string {
    return std::visit(
        []<typename var>(var&& arg) { return std::to_string(static_cast<uint16_t>(std::forward<decltype(arg)>(arg))); },
        port);
  }
};
}  // namespace tfc::mqtt::config
