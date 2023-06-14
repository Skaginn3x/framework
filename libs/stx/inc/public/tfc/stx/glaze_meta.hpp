#pragma once
#include <chrono>
#include <ratio>
#include <sstream>
#include <string_view>
#include <type_traits>

#include <date/date.h>
#include <fmt/chrono.h>
#include <glaze/glaze.hpp>

#include <tfc/stx/string_view_join.hpp>

namespace tfc::detail {
template <std::intmax_t num, std::intmax_t den>
inline constexpr auto make_ratio_symbol() -> std::string_view;

template <std::intmax_t num, std::intmax_t den>
static constexpr auto make_name() -> std::string_view;

template <typename rep_t, typename period_t>
struct duration_hack {
  rep_t rep{};
};
}  // namespace tfc::detail

template <std::intmax_t num, std::intmax_t den>
struct glz::meta<std::ratio<num, den>> {
  using type = std::ratio<num, den>;
  static constexpr auto value{ glz::object("numerator", &type::num, "denominator", &type::den) };
  static constexpr auto name{ tfc::detail::make_name<num, den>() };
};
template <typename rep_t, typename period_t>
struct glz::meta<tfc::detail::duration_hack<rep_t, period_t>> {
  using type = tfc::detail::duration_hack<rep_t, period_t>;
  static constexpr std::string_view unit_symbol{ "s" };
  static constexpr std::string_view unit_ratio{ tfc::detail::make_ratio_symbol<period_t::num, period_t::den>() };
  static constexpr std::string_view unit{ tfc::stx::string_view_join_v<unit_ratio, unit_symbol> };
  static constexpr std::string_view dimension{ "time" };
  static constexpr auto ratio{ period_t{} };
  static constexpr auto value{ glz::object(
      "value",
      &type::rep,
      "unit",
      [](auto&&) -> auto const& { return unit; },
      "dimension",
      [](auto&&) -> auto const& { return dimension; },
      "ratio",
      [](auto&&) -> auto const& { return ratio; }) };
  static constexpr std::string_view prefix{ "std::chrono::duration<" };
  static constexpr std::string_view postfix{ ">" };
  static constexpr std::string_view separator{ "," };
  static constexpr auto name{
    tfc::stx::string_view_join_v<prefix, glz::name_v<rep_t>, separator, glz::name_v<period_t>, postfix>
  };
};
template <typename rep_t, typename period_t>
struct glz::meta<std::chrono::duration<rep_t, period_t>> {
  static constexpr std::string_view prefix{ "std::chrono::duration<" };
  static constexpr std::string_view postfix{ ">" };
  static constexpr std::string_view separator{ "," };
  static constexpr auto name{
    tfc::stx::string_view_join_v<prefix, glz::name_v<rep_t>, separator, glz::name_v<period_t>, postfix>
  };
};

namespace glz::detail {
inline auto parse8601(const std::string& save) -> date::sys_time<std::chrono::milliseconds> {
  std::istringstream in{ save };
  date::sys_time<std::chrono::milliseconds> tp;
  in >> date::parse("%FT%TZ", tp);
  if (in.fail()) {
    in.clear();
    in.exceptions(std::ios::failbit);
    in.str(save);
    in >> date::parse("%FT%T%Ez", tp);
  }
  return tp;
}
template <>
struct from_json<std::chrono::time_point<std::chrono::system_clock>> {
  template <auto Opts>
  static void op(auto& value, auto&&... args) {
    std::string rep;
    read<json>::op<Opts>(rep, args...);
    value = parse8601(rep);
  }
};

template <>
struct to_json<std::chrono::time_point<std::chrono::system_clock>> {
  template <auto Opts>
  static void op(auto& value, auto&&... args) noexcept {
    std::stringstream str_stream;
    str_stream << date::format("%FT%T%Ez", value);
    write<json>::op<Opts>(str_stream.str(), args...);
  }
};

template <typename rep_t, typename period_t>
struct from_json<std::chrono::duration<rep_t, period_t>> {
  template <auto opts>
  static void op(std::chrono::duration<rep_t, period_t>& value, auto&&... args) {
    tfc::detail::duration_hack<rep_t, period_t> substitute{};
    read<json>::op<opts>(substitute, args...);
    value = std::chrono::duration<rep_t, period_t>{ substitute.rep };
  }
};
template <typename rep_t, typename period_t>
struct to_json<std::chrono::duration<rep_t, period_t>> {
  template <auto opts>
  static void op(auto& value, auto&&... args) noexcept {
    tfc::detail::duration_hack<rep_t, period_t> substitute{ .rep = value.count() };
    write<json>::op<opts>(substitute, args...);
  }
};

}  // namespace glz::detail

namespace tfc::detail {
template <std::intmax_t num, std::intmax_t den>
inline constexpr auto make_ratio_symbol() -> std::string_view {
  using type = std::ratio<num, den>;
  if constexpr (std::is_same_v<type, std::atto>) {
    return "a";
  } else if constexpr (std::is_same_v<type, std::femto>) {
    return "f";
  } else if constexpr (std::is_same_v<type, std::pico>) {
    return "p";
  } else if constexpr (std::is_same_v<type, std::nano>) {
    return "n";
  } else if constexpr (std::is_same_v<type, std::micro>) {
    return "μ";
  } else if constexpr (std::is_same_v<type, std::milli>) {
    return "m";
  } else if constexpr (std::is_same_v<type, std::centi>) {
    return "c";
  } else if constexpr (std::is_same_v<type, std::deci>) {
    return "d";
  } else if constexpr (std::is_same_v<type, std::deca>) {
    return "da";
  } else if constexpr (std::is_same_v<type, std::hecto>) {
    return "h";
  } else if constexpr (std::is_same_v<type, std::kilo>) {
    return "k";
  } else if constexpr (std::is_same_v<type, std::mega>) {
    return "M";
  } else if constexpr (std::is_same_v<type, std::giga>) {
    return "G";
  } else if constexpr (std::is_same_v<type, std::tera>) {
    return "T";
  } else if constexpr (std::is_same_v<type, std::peta>) {
    return "P";
  } else if constexpr (std::is_same_v<type, std::exa>) {
    return "E";
  } else {
    []<bool flag = false>() {
      static_assert(flag, "Missing ratio symbol, please add it to the list.");
    }
    ();
  }
}

template <std::intmax_t num, std::intmax_t den>
static constexpr auto make_name() -> std::string_view {
  using type = std::ratio<num, den>;
  if constexpr (std::is_same_v<type, std::atto>) {
    return "std::atto";
  } else if constexpr (std::is_same_v<type, std::femto>) {
    return "std::femto";
  } else if constexpr (std::is_same_v<type, std::pico>) {
    return "std::pico";
  } else if constexpr (std::is_same_v<type, std::nano>) {
    return "std::nano";
  } else if constexpr (std::is_same_v<type, std::micro>) {
    return "std::micro";
  } else if constexpr (std::is_same_v<type, std::milli>) {
    return "std::milli";
  } else if constexpr (std::is_same_v<type, std::centi>) {
    return "std::centi";
  } else if constexpr (std::is_same_v<type, std::deci>) {
    return "std::deci";
  } else if constexpr (std::is_same_v<type, std::deca>) {
    return "std::deca";
  } else if constexpr (std::is_same_v<type, std::hecto>) {
    return "std::hecto";
  } else if constexpr (std::is_same_v<type, std::kilo>) {
    return "std::kilo";
  } else if constexpr (std::is_same_v<type, std::mega>) {
    return "std::mega";
  } else if constexpr (std::is_same_v<type, std::giga>) {
    return "std::giga";
  } else if constexpr (std::is_same_v<type, std::tera>) {
    return "std::tera";
  } else if constexpr (std::is_same_v<type, std::peta>) {
    return "std::peta";
  } else if constexpr (std::is_same_v<type, std::exa>) {
    return "std::exa";
  } else {
    []<bool flag = false>() {
      static_assert(flag, "Missing ratio name, please add it to the list.");
    }
    ();
  }
}
}  // namespace tfc::detail
