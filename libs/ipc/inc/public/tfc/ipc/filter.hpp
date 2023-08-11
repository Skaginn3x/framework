#pragma once
#include <concepts>

#include <tfc/ipc.hpp>

namespace tfc::ipc::filter {

namespace concepts {
template <typename type>
concept is_slot = requires {
  // clang-format off
  std::same_as<bool_slot, std::remove_cvref_t<type>>
  || std::same_as<int_slot, std::remove_cvref_t<type>>
  || std::same_as<uint_slot, std::remove_cvref_t<type>>
  || std::same_as<double_slot, std::remove_cvref_t<type>>
  || std::same_as<string_slot, std::remove_cvref_t<type>>
  || std::same_as<json_slot, std::remove_cvref_t<type>>
  ;
  // clang-format on
};
}

template <concepts::is_slot type, typename... filters>
struct filtered_slot {
  filtered_slot(auto&&... args) : slot{ std::forward<decltype(args)>(args)... } {}
  
  type slot_;
};

}  // namespace tfc::ipc::filter
