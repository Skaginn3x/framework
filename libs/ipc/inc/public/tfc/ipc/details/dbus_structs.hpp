#pragma once

#include <chrono>
#include <string>

#include <tfc/ipc/enums.hpp>

namespace tfc::ipc_ruler {

struct signal {
  std::string name;
  ipc::details::type_e type;
  std::string created_by;
  std::chrono::time_point<std::chrono::system_clock> created_at;
  std::chrono::time_point<std::chrono::system_clock> last_registered;
  std::string description;
};

struct slot {
  std::string name;
  ipc::details::type_e type;
  std::string created_by;
  std::chrono::time_point<std::chrono::system_clock> created_at;
  std::chrono::time_point<std::chrono::system_clock> last_registered;
  std::chrono::time_point<std::chrono::system_clock> last_modified;
  std::string modified_by;
  std::string connected_to;
  std::string description;
};

}  // namespace tfc::ipc_ruler
