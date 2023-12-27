#pragma once

#include <string>
#include <vector>

namespace tfc::global {

auto set_interfaces() -> void;
auto get_interfaces() -> std::vector<std::string>;

}  // namespace tfc::global
