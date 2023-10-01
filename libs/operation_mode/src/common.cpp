#include <tfc/operation_mode/common.hpp>

#include <magic_enum.hpp>

namespace tfc::operation {

[[nodiscard]] auto enum_name(mode_e enum_value) -> std::string_view {
  return magic_enum::enum_name(enum_value);
}
[[nodiscard]] auto enum_cast(std::string_view enum_name) -> std::optional<mode_e> {
  return magic_enum::enum_cast<mode_e>(enum_name);
}

}  // namespace tfc::operation
