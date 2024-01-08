#pragma once

#include <string>
#include <vector>
#include <chrono>

namespace tfc::ec::common {

/// \brief The scan time for the ethercat network, between each poll.
static constexpr auto cycle_time() {
  // todo make a compile option
  return std::chrono::milliseconds(1);
};

/// \return The network interfaces on the running hardware.
auto get_interfaces() -> std::vector<std::string> const&;

}  // namespace tfc::ec::common
