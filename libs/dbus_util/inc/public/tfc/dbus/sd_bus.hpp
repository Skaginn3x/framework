#pragma once

import std;

#include <systemd/sd-bus.h>

namespace tfc::dbus {
static inline auto sd_bus_open_system() -> sd_bus* {
  sd_bus* bus = nullptr;
  if (sd_bus_open_system(&bus) < 0) {
    throw std::runtime_error(std::string{ "Unable to open sd-bus, error: " } + strerror(errno));
  }
  return bus;
}

}  // namespace tfc::dbus
