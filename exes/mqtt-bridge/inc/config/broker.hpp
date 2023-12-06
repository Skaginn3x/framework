#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include <glaze/core/common.hpp>

#include <structs.hpp>

template <>
struct glz::meta<tfc::mqtt::structs::ssl_active_e> {
  using enum tfc::mqtt::structs::ssl_active_e;
  static constexpr std::string_view name{ "tfc::mqtt::ssl" };
  static constexpr auto value = glz::enumerate("yes", yes, "no", no);
};

namespace tfc::mqtt::config {

struct broker {
  std::string address{};
  uint16_t port{};
  structs::ssl_active_e ssl_active{};
  std::string username{};
  std::string password{};
  std::string client_id{};

  struct glaze {
    // clang-format off
             static constexpr auto value{glz::object(
                     "address", &broker::address, "Hostname or IP address of the MQTT broker",
                     "port", &broker::port, "Port of the MQTT broker.",
                     "ssl_active", &broker::ssl_active, "Whether or not to use SSL to connect to the MQTT broker",
                     "username", &broker::username, "Username for the MQTT broker",
                     "password", &broker::password, "Password for the MQTT broker",
                     "client_id", &broker::client_id, "Client ID, used to identify which client is sending information"
             )};
    // clang-format on
    static constexpr std::string_view name{ "broker" };
  };

  [[nodiscard]] auto get_port() const -> std::string { return std::to_string(static_cast<int>(port)); }
};

}  // namespace tfc::mqtt::config
