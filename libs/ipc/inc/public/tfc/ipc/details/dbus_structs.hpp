#pragma once

#include <chrono>
#include <string>

#include <tfc/ipc/enums.hpp>

namespace tfc::ipc_ruler {

using time_point_t = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;

struct signal {
  std::string name;
  ipc::details::type_e type;
  std::string created_by; // dbus_service_name
  time_point_t created_at;
  time_point_t last_registered;
  std::string description;
};

struct slot {
  std::string name;
  ipc::details::type_e type;
  std::string created_by; // dbus_service_name
  time_point_t created_at;
  time_point_t last_registered;
  time_point_t last_modified;
  std::string modified_by;
  std::string connected_to;
  std::string description;
};

}  // namespace tfc::ipc_ruler
