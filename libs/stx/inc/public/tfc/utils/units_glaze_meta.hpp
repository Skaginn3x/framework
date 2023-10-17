#pragma once

#include <glaze/core/common.hpp>
#include <glaze/core/meta.hpp>
#include <string_view>

#include <mp-units/bits/ratio.h>
#include <mp-units/quantity.h>
#include <mp-units/systems/angular/angular.h>
#include <mp-units/unit.h>

#include <tfc/stx/string_view_join.hpp>
#include <tfc/utils/json_schema.hpp>

namespace tfc::unit {
template <mp_units::Reference auto ref_t>
inline constexpr auto dimension_name() -> std::string_view;
}  // namespace tfc::unit

template <>
struct glz::meta<mp_units::ratio> {
  static constexpr auto value{ glz::object("numerator", &mp_units::ratio::num, "denominator", &mp_units::ratio::den) };
  static constexpr auto name{ "units::ratio" };
};

template <typename CharT, mp_units::Unit U, mp_units::unit_symbol_formatting fmt>
struct const_unit_symbol {
  static consteval auto unit_symbol_len() -> std::size_t { return unit_symbol(U{}, fmt).size(); }
  static constexpr auto impl() noexcept {
    std::array<CharT, unit_symbol_len() + 1> buffer{};
    auto foo = unit_symbol(U{}, fmt);
    std::ranges::copy(std::begin(foo), std::end(foo), std::begin(buffer));
    return buffer;
  }
  // Give the joined string static storage
  static constexpr auto arr = impl();
  // View as a std::string_view
  static constexpr std::basic_string_view<CharT> value{ arr.data(), arr.size() - 1 };
};

template <typename CharT = char, mp_units::Unit U, mp_units::unit_symbol_formatting fmt = mp_units::unit_symbol_formatting{}>
[[nodiscard]] constexpr std::basic_string_view<CharT> unit_symbol_view(U) {
  return const_unit_symbol<CharT, U, fmt>::value;
}

template <mp_units::Reference auto ref_t, typename rep_t>
struct glz::meta<mp_units::quantity<ref_t, rep_t>> {
  using type = mp_units::quantity<ref_t, rep_t>;
  static constexpr std::string_view unit{
    unit_symbol_view<char, decltype(ref_t), mp_units::unit_symbol_formatting{ .encoding = mp_units::text_encoding::ascii }>(
        ref_t)
  };
  static constexpr auto dimension{ tfc::unit::dimension_name<ref_t>() };
  static auto constexpr value{ [](auto&& self) -> auto& { return self.numerical_value_; } };
  static std::string_view constexpr prefix{ "units::quantity<" };
  static std::string_view constexpr postfix{ ">" };
  static std::string_view constexpr separator{ "," };
  static auto constexpr name{
    tfc::stx::string_view_join_v<prefix, dimension, separator, separator, unit, glz::name_v<rep_t>, postfix>
  };
};

namespace tfc::json::detail {

template <typename value_t>
struct to_json_schema;

template <mp_units::Reference auto ref_t, typename rep_t>
struct to_json_schema<mp_units::quantity<ref_t, rep_t>> {
  static constexpr std::string_view unit{ decltype(ref_t)::symbol.ascii() };
  static constexpr mp_units::ratio ratio{ mp_units::as_ratio(ref_t) };
  static constexpr auto dimension{ tfc::unit::dimension_name<ref_t>() };
  template <auto opts>
  static void op(auto& schema, auto& defs) {
    auto& data = schema.attributes.tfc_metadata;
    if (!data.has_value()) {
      data = tfc::json::schema_meta{};
    }
    data->unit = unit;
    data->dimension = dimension;
    if constexpr (mp_units::Magnitude<decltype(ref_t)>) {
      data->ratio = tfc::json::schema_meta::ratio_impl{ .numerator = ratio.num, .denominator = ratio.den };
    }
    to_json_schema<rep_t>::template op<opts>(schema, defs);
  }
};

}  // namespace tfc::json::detail

namespace tfc::unit {
template <mp_units::Reference auto ref_t>
inline constexpr auto dimension_name() -> std::string_view {
  if constexpr (mp_units::convertible(ref_t, mp_units::si::metre)) {
    return "length";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::hertz)) {
    return "frequency";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::ampere)) {
    return "electric_current";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::volt)) {
    return "voltage";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::watt)) {
    return "power";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::gram)) {
    return "mass";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::litre)) {
    return "volume";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::angular::degree)) {
    return "angular degree";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::square(mp_units::si::metre))) {
    return "area";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::metre / mp_units::si::second)) {
    return "speed";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::metre / mp_units::square(mp_units::si::second))) {
    return "acceleration";
  } else {
    []<bool flag = false>() {
      static_assert(flag, "Missing dimension name, please add it to the list.");
    }
    ();
  }
}
}  // namespace tfc::unit
