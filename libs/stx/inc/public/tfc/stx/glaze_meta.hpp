#pragma once
#include <chrono>
#include <expected>
#include <optional>
#include <ratio>
#include <sstream>
#include <string_view>
#include <type_traits>

#include <date/date.h>
#include <fmt/chrono.h>

#include <glaze/glaze.hpp>

#include <tfc/stx/basic_fixed_string.hpp>
#include <tfc/stx/string_view_join.hpp>
#include <tfc/stx/to_string_view.hpp>
#include <tfc/utils/json_schema.hpp>

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
  } else if constexpr (std::is_same_v<type, std::ratio<60>>) {
    return "ratio<60>";
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
  } else if constexpr (num == den) {
    // example std::chrono::seconds
    return "none";
  } else {
    []<bool flag = false>() {
      static_assert(flag, "Missing ratio name, please add it to the list.");
    }
    ();
  }
}

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
  static constexpr auto value{ &type::rep };
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

template <>
struct glz::meta<std::chrono::system_clock> {
  static constexpr std::string_view name{ "std::chrono::system_clock" };
};

template <typename clock_t, typename duration_t>
struct glz::meta<std::chrono::time_point<clock_t, duration_t>> {
  static constexpr std::string_view prefix{ "std::chrono::time_point<" };
  static constexpr std::string_view postfix{ ">" };
  static constexpr std::string_view separator{ "," };
  static constexpr auto name{
    tfc::stx::string_view_join_v<prefix, glz::name_v<clock_t>, separator, glz::name_v<duration_t>, postfix>
  };
};

template <typename char_type, unsigned len>
struct glz::meta<tfc::stx::basic_fixed_string<char_type, len>> {
  static constexpr std::string_view prefix{ "basic_fixed_string<" };
  static constexpr std::string_view postfix{ ">" };
  static constexpr std::string_view separator{ "," };
  static constexpr auto name{
    tfc::stx::string_view_join_v<prefix, glz::name_v<char_type>, separator, tfc::stx::to_string_view_v<len>, postfix>
  };
};

namespace glz::detail {
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
  static void op(auto&& value, auto&&... args) noexcept {
    tfc::detail::duration_hack<rep_t, period_t> substitute{ .rep = value.count() };
    write<json>::op<opts>(substitute, args...);
  }
};

template <typename duration_t>
  requires(!std::is_same_v<duration_t, std::chrono::microseconds> && !std::is_same_v<duration_t, std::chrono::nanoseconds>)
constexpr auto parse8601(const std::string& save) -> date::sys_time<duration_t> {
  std::istringstream in{ save };
  date::sys_time<duration_t> tp;
  date::from_stream(in, "%FT%TZ", tp);
  if (in.fail()) {
    in.clear();
    in.exceptions(std::ios::failbit);
    in.str(save);
    date::from_stream(in, "%FT%T%Ez", tp);
  }
  return tp;
}

template <typename clock_t, typename duration_t>
struct from_json<std::chrono::time_point<clock_t, duration_t>> {
  template <auto opts>
  static void op([[maybe_unused]] auto& value, auto&&... args) {
    std::string rep;
    read<json>::op<opts>(rep, args...);
    value = parse8601<duration_t>(rep);
  }
};

template <typename clock_t, typename duration_t>
struct to_json<std::chrono::time_point<clock_t, duration_t>> {
  template <auto opts>
  static void op(auto& value, auto&&... args) noexcept {
    std::string iso8601{ fmt::format("{:%FT%T%z}", value) };
    write<json>::op<opts>(iso8601, args...);
  }
};

template <typename char_type, unsigned len>
struct from_json<tfc::stx::basic_fixed_string<char_type, len>> {
  using type = tfc::stx::basic_fixed_string<char_type, len>;

  template <auto opts>
  static void op(type& value, auto&&... args) {
    std::basic_string<char_type> substitute{};
    read<json>::op<opts>(substitute, args...);
    std::size_t cnt{};
    for (auto const& character : substitute) {
      if (cnt >= len) {
        break;
      }
      value.at(cnt++) = character;
    }
  }
};

template <typename char_type, unsigned len>
struct to_json<tfc::stx::basic_fixed_string<char_type, len>> {
  template <auto opts>
  static void op(auto& value, auto&&... args) noexcept {
    write<json>::op<opts>(value.view(), std::forward<decltype(args)>(args)...);
  }
};
}  // namespace glz::detail

namespace tfc::json::detail {
template <typename value_t>
struct to_json_schema;

template <typename clock_t, typename duration_t>
struct to_json_schema<std::chrono::time_point<clock_t, duration_t>> {
  template <auto opts>
  static void op(auto& schema, auto&) {
    // fix in https://github.com/Skaginn3x/framework/issues/555
    // using enum tfc::json::defined_formats;
    // schema.attributes.format = datetime;
    schema.type = { "string" };
  }
};

template <typename rep_t, typename period_t>
struct to_json_schema<std::chrono::duration<rep_t, period_t>> {
  [[maybe_unused]] static constexpr std::string_view unit_symbol{ "s" };
  [[maybe_unused]] static constexpr std::string_view unit_ratio{
    tfc::detail::make_ratio_symbol<period_t::num, period_t::den>()
  };
  static constexpr std::string_view unit{ tfc::stx::string_view_join_v<unit_ratio, unit_symbol> };

  template <auto opts>
  static void op(auto& schema, auto& defs) {
    // fix in https://github.com/Skaginn3x/framework/issues/555
    // auto& data = schema.attributes.tfc_metadata;
    // if (!data.has_value()) {
    //   data = tfc::json::schema_meta{};
    // }
    // data->unit = schema_meta::unit_meta{ .unit_ascii = unit, .unit_unicode = unit };
    // data->dimension = "time";
    // data->ratio = tfc::json::schema_meta::ratio_impl{ .numerator = period_t::num, .denominator = period_t::den };
    to_json_schema<rep_t>::template op<opts>(schema, defs);
  }
};

template <typename rep_t>
struct to_json_schema<std::optional<rep_t>> {
  template <auto opts>
  static void op(auto& schema, auto& defs) {
    // fix in https://github.com/Skaginn3x/framework/issues/555
    // auto& data = schema.attributes.tfc_metadata;
    // if (!data.has_value()) {
    //   data = tfc::json::schema_meta{};
    // }
    // data->required = false;
    to_json_schema<rep_t>::template op<opts>(schema, defs);
  }
};

template <typename value_t, typename error_t>
struct to_json_schema<std::expected<value_t, error_t>> {
  template <auto opts>
  static void op(auto& schema, auto& defs) {
    to_json_schema<std::variant<value_t, error_t>>::template op<opts>(schema, defs);
  }
};

}  // namespace tfc::json::detail
