#pragma once

#include <type_traits>

namespace tfc::stx {

template <typename func, typename... args>
concept nothrow_invocable = requires { std::is_nothrow_invocable_v<func, args...>; };

}
