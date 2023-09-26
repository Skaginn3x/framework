#pragma once

#include <string>

#include <tfc/ipc/enums.hpp>
#include <tfc/stx/millisecond_clock.hpp>

namespace tfc::ipc_ruler {

struct signal {
  std::string name;
  ipc::details::type_e type;
  std::string created_by;
  tfc::stx::millisecond_system_clock::time_point created_at;
  tfc::stx::millisecond_system_clock::time_point last_registered;
  std::string description;
};

struct slot {
  std::string name;
  ipc::details::type_e type;
  std::string created_by;
  tfc::stx::millisecond_system_clock::time_point created_at;
  tfc::stx::millisecond_system_clock::time_point last_registered;
  tfc::stx::millisecond_system_clock::time_point last_modified;
  std::string modified_by;
  std::string connected_to;
  std::string description;
};

}  // namespace tfc::ipc_ruler
