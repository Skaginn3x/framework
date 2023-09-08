#pragma once

#include <any>
#include <optional>

#include <tfc/ipc.hpp>

namespace tfc::mqtt::structs {

enum struct ssl_active_e { yes, no };

struct signal_data {
  tfc::ipc_ruler::signal information;
  tfc::ipc::details::any_slot receiver;
  std::optional<std::any> current_value;
};

}  // namespace tfc::mqtt::structs
