#pragma once

#include <type_traits>

#define EXPORT __attribute__((visibility("default")))

namespace tfc::stx {

template <typename func, typename... args>
concept nothrow_invocable = requires { std::is_nothrow_invocable_v<func, args...>; };

template <typename enum_t>
concept is_enum = requires { std::is_enum_v<enum_t>; };

}  // namespace tfc::stx
