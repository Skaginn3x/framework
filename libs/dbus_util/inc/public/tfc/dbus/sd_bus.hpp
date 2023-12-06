#pragma once

#include <cerrno>
#include <exception>
#include <string>

#include <systemd/sd-bus.h>

namespace tfc::dbus {
static inline auto sd_bus_open_system() -> sd_bus* {
  sd_bus* bus = nullptr;
  if (sd_bus_open_system(&bus) < 0) {
    throw std::runtime_error(std::string{ "Unable to open sd-bus, error: " } + strerror(errno));
  }
  return bus;
}

static inline auto sd_bus_open_system_mon() -> sd_bus* {
  sd_bus* bus = nullptr;
  if (sd_bus_open_system(&bus) < 0) {
    throw std::runtime_error(std::string{ "Unable to open sd-bus, error: " } + strerror(errno));
  }
  sd_bus_set_monitor(bus, true);
  return bus;
}

}  // namespace tfc::dbus
