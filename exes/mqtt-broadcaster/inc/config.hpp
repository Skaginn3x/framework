#pragma once

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>

// File under /etc/tfc/mqtt-broadcaster/def/mqtt_broadcaster.json which lists the signals that are supposed to be published
// on the mqtt broker. Updating the signals in this file will cause the running program to stop and restart
struct config {
  tfc::confman::observable<std::vector<std::string>> _allowed_topics{};
  struct glaze {
    static constexpr auto value{ glz::object("_allowed_topics", &config::_allowed_topics) };
    static constexpr auto name{ "mqtt_broadcaster" };
  };
};