#pragma once

#include <cstddef>
#include <span>
#include <string_view>

#include <fmt/core.h>

namespace tfc::snitch::detail {

template <typename Char>
struct custom_handler {
  constexpr void on_text(const Char*, const Char*) {}

  constexpr auto on_arg_id() -> int {
    num_args++;
    all_named = false;
    return 0;
  }

  constexpr auto on_arg_id([[maybe_unused]] int id) -> int {
    num_args++;
    all_named = false;
    return 0;
  }

  constexpr auto on_arg_id([[maybe_unused]] fmt::basic_string_view<Char> id) -> int {
    num_args++;
    return 0;
  }

  constexpr void on_replacement_field([[maybe_unused]] int id, [[maybe_unused]] const Char* begin) {
    // todo
  }

  constexpr auto on_format_specs([[maybe_unused]] int id, [[maybe_unused]] const Char* begin, [[maybe_unused]] const Char*)
      -> const Char* {
    has_format_specs = true;
    return begin;
  }

  constexpr void on_error([[maybe_unused]] const char* msg) {
    // todo
  }
  std::size_t num_args{ 0 };
  bool all_named{ true };
  bool has_format_specs{ false };
};

template <typename Char>
consteval auto check_all_arguments_named(std::basic_string_view<Char> format_str) -> bool {
  custom_handler<Char> handler;
  fmt::detail::parse_format_string<true>(fmt::basic_string_view<Char>{ format_str.data(), format_str.size() }, handler);
  return handler.all_named;
}

template <typename Char>
consteval auto check_all_arguments_no_format(std::basic_string_view<Char> format_str) -> bool {
  custom_handler<Char> handler;
  fmt::detail::parse_format_string<true>(fmt::basic_string_view<Char>{ format_str.data(), format_str.size() }, handler);
  return !handler.has_format_specs;
}

template <typename Char>
consteval auto arg_count(std::basic_string_view<Char> format_str) -> std::size_t {
  custom_handler<Char> handler;
  fmt::detail::parse_format_string<true>(fmt::basic_string_view<Char>{ format_str.data(), format_str.size() }, handler);
  return handler.num_args;
}

template <typename Char, std::size_t N>
struct custom_handler_names {
  constexpr void on_text(const Char*, const Char*) {}

  constexpr auto on_arg_id() -> int { return 0; }

  constexpr auto on_arg_id([[maybe_unused]] int id) -> int { return 0; }

  constexpr auto on_arg_id(fmt::basic_string_view<Char> id) -> int {
    if (auto it{ std::ranges::find_if(names, [id](const auto& name) { return name == id; }) }; it == names.end()) {
      names[idx++] = { id.data(), id.size() };
    }
    return 0;
  }

  constexpr void on_replacement_field([[maybe_unused]] int id, [[maybe_unused]] const Char* begin) {
    // todo
  }

  constexpr auto on_format_specs([[maybe_unused]] int id, const Char* begin, const Char*) -> const Char* { return begin; }

  constexpr void on_error([[maybe_unused]] const char* msg) {
    // todo
  }
  std::array<std::basic_string_view<Char>, N> names{};
  std::size_t idx{};
};

template <std::string_view const& format_str>
struct arg_names_impl {
  static constexpr auto unique_names() noexcept {
    using Char = char;
    custom_handler_names<Char, num_args> handler;
    fmt::detail::parse_format_string<true>(fmt::basic_string_view<Char>{ format_str.data(), format_str.size() }, handler);
    return handler.idx;
  }
  static constexpr auto impl() noexcept {
    using Char = char;
    custom_handler_names<Char, num_args> handler;
    fmt::detail::parse_format_string<true>(fmt::basic_string_view<Char>{ format_str.data(), format_str.size() }, handler);
    return handler.names;
  }
  static constexpr auto num_args = arg_count(format_str);
  static constexpr auto arr = impl();
  static constexpr auto size = unique_names();
  static constexpr std::span<const std::string_view, size> value{ arr.data(), size };
};

template <std::string_view const& format_str>
consteval auto arg_names() -> decltype(auto) {
  return arg_names_impl<format_str>::value;
}

}  // namespace tfc::snitch::detail
