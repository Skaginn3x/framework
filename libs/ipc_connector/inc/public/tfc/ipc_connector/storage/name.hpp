#pragma once

#pragma once

#include <string>
#include <string_view>

#include <glaze/core/common.hpp>

#include <tfc/confman/read_only.hpp>
#include <tfc/stx/basic_fixed_string.hpp>
#include <tfc/stx/string_view_join.hpp>

namespace tfc::ipc::storage {

using std::string_view_literals::operator""sv;

template <std::string_view const& signal_slot, std::string_view const& type_name>
struct name {
  tfc::confman::read_only<std::string> value{};

  struct glaze {
    static constexpr std::string_view delimiter{ "." };
    static constexpr std::string_view name{ tfc::stx::string_view_join_v<signal_slot, delimiter, type_name> };
    static constexpr auto value{ glz::object("name", &name::value) };
  };
};

}  // namespace tfc::ipc::storage
