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

template <typename unit_t>
struct unit_symbol {
  static constexpr std::size_t size{ 255 };
  static constexpr auto impl() noexcept {
    std::array<char, size> array{};
    auto out_it{ mp_units::unit_symbol_to(array.begin(), unit_t{}) };
    auto len = std::distance(array.begin(), out_it);
    return std::make_pair( array, len );
  }
  // Give the joined string static storage
  static constexpr auto arr = impl();
  static_assert(arr.second < size);
  // View as a std::string_view
  static constexpr std::string_view value{ arr.first.data(), arr.second };
};
// Helper to get the value out
template <typename unit_t>
static constexpr auto unit_symbol_v = unit_symbol<unit_t>::value;

template <mp_units::Reference auto ref_t, typename rep_t>
struct glz::meta<mp_units::quantity<ref_t, rep_t>> {
  using type = mp_units::quantity<ref_t, rep_t>;
  static constexpr std::string_view unit{ unit_symbol_v<decltype(ref_t)> };
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
    return "metre";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::hertz)) {
    return "hertz";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::ampere)) {
    return "ampere";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::volt)) {
    return "voltage";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::watt)) {
    return "watt";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::gram)) {
    return "gram";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::litre)) {
    return "litre";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::angular::degree)) {
    return "degree";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::square(mp_units::si::milli<mp_units::si::metre>))) {
    return "millimetre^2";
  } else {
    []<bool flag = false>() {
      static_assert(flag, "Missing dimension name, please add it to the list.");
    }
    ();
  }
}
}  // namespace tfc::unit
