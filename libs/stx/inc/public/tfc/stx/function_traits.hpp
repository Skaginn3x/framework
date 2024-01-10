#pragma once

namespace tfc::stx {

template <typename signature_t>
struct function_traits;

template <typename return_t, typename... args_t>
struct function_traits<return_t(args_t...)> {
  static constexpr std::size_t arity = sizeof...(args_t);
};

template <std::size_t nth_arg, typename signature_t>
struct function_traits_n;

template <std::size_t nth_arg, typename return_t, typename... args_t>
struct function_traits_n<nth_arg, return_t(args_t...)> {
  using type = typename std::tuple_element_t<nth_arg, std::tuple<args_t...>>;
};

template <std::size_t nth_arg, typename signature_t>
using function_traits_n_t = typename function_traits_n<nth_arg, signature_t>::type;

}
