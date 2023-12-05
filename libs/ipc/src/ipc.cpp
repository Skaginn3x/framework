#include "tfc/ipc.hpp"

#include <magic_enum.hpp>

namespace tfc::ipc::details {

auto enum_name(mass_error_e err) -> std::string_view {
  return magic_enum::enum_name(err);
}

}  // namespace tfc::ipc::details
