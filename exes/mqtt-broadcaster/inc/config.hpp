#pragma once

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>

// File under /etc/tfc/mqtt-broadcaster/def/mqtt_broadcaster.json which lists the signals that are supposed to be published
// on the mqtt broker. Updating the signals in this file will cause the running program to stop and restart
struct config {
  std::string address{};
  std::string port{};
  std::string username{};
  std::string password{};
  std::string node_id{};
  std::string group_id{};
  std::string client_id{};

  struct glaze {
    // clang-format off
    static constexpr auto value{ glz::object(
        "address", &config::address,
        "port", &config::port,
        "username", &config::username,
        "password", &config::password,
        "node_id", &config::node_id,
        "group_id", &config::group_id,
        "client_id", &config::client_id
      ) };
    // clang-format on
    static constexpr auto name{ "mqtt_broadcaster" };
  };
};
