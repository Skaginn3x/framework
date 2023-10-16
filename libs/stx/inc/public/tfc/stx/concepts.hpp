#pragma once

#include <concepts>
#include <type_traits>

namespace tfc::stx {

template <typename func, typename... args>
concept nothrow_invocable = requires { std::is_nothrow_invocable_v<func, args...>; };

template <typename func, typename... args>
concept invocable = std::invocable<std::remove_cvref_t<func>, args...>;

template <typename enum_t>
concept is_enum = requires { requires std::is_enum_v<enum_t>; };

template <typename compare_to_t, typename... args_t>
concept is_one_of = requires { requires(std::is_same_v<compare_to_t, args_t> || ...); };

// from
// https://stackoverflow.com/questions/16337610/how-to-know-if-a-type-is-a-specialization-of-stdvector
template <class, template <class...> class>
inline constexpr bool is_specialization_v = false;

template <template <class...> class type, class... args>
inline constexpr bool is_specialization_v<type<args...>, type> = true;

}  // namespace tfc::stx
