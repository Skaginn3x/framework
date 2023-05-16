#pragma once

#include <cerrno>
#include <exception>
#include <string>

#include <systemd/sd-bus.h>

namespace tfc::dbus {
static auto sd_bus_open_system() -> sd_bus* {
  sd_bus* bus = nullptr;
  if (sd_bus_open(&bus) < 0) {
    throw std::runtime_error(std::string{ "Unable to open sd-bus, error: " } + strerror(errno));
  }
  return bus;
}

}  // namespace tfc::dbus
