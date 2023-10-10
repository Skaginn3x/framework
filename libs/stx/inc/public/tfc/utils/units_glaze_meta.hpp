#pragma once

#include <cstdint>
#include <glaze/core/meta.hpp>
#include <string_view>

#include <mp-units/quantity.h>
#include <mp-units/systems/angular/angular.h>
#include <mp-units/systems/isq/isq.h>
#include <glaze/glaze.hpp>

#include <tfc/stx/string_view_join.hpp>
#include <tfc/utils/json_schema.hpp>

namespace tfc::unit {
template <mp_units::Reference auto ref_t>
inline constexpr auto dimension_name() -> std::string_view;
}  // namespace tfc::unit

// template <>
// struct glz::meta<units::ratio> {
//   static constexpr auto value{ glz::object("numerator", &units::ratio::num, "denominator", &units::ratio::den) };
//   static constexpr auto name{ "units::ratio" };
// };

template <mp_units::Reference auto ref_t, typename rep_t>
struct glz::meta<mp_units::quantity<ref_t, rep_t>> {
  using type = mp_units::quantity<ref_t, rep_t>;
  // static constexpr std::string_view unit{ unit_t::symbol.standard().data_ };
  // static constexpr auto dimension{ tfc::unit::dimension_name<dimension_t>() };
  static auto constexpr value{ [](auto&& self) -> auto& { return self.numerical_value_; } };
  static std::string_view constexpr prefix{ "units::quantity<" };
  static std::string_view constexpr postfix{ ">" };
  static std::string_view constexpr separator{ "," };
  static auto constexpr name{ tfc::stx::string_view_join_v<prefix, separator, separator, glz::name_v<rep_t>, postfix> };
};

namespace tfc::json::detail {

template <typename value_t>
struct to_json_schema;

template <mp_units::Reference auto ref_t, typename rep_t>
struct to_json_schema<mp_units::quantity<ref_t, rep_t>> {
  static constexpr std::string_view unit{ decltype(ref_t)::symbol.ascii() };
  // static constexpr units::ratio ratio{ units::as_ratio(unit_t::mag) };
  static constexpr auto dimension{ tfc::unit::dimension_name<ref_t>() };
  template <auto opts>
  static void op(auto& schema, auto& defs) {
    auto& data = schema.attributes.tfc_metadata;
    if (!data.has_value()) {
      data = tfc::json::schema_meta{};
    }
    data->unit = unit;
    data->dimension = dimension;
    // data->ratio = tfc::json::schema_meta::ratio_impl{ .numerator = ratio.num, .denominator = ratio.den };
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
    return "hertz";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::ampere)) {
    return "ampere";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::volt)) {
    return "voltage";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::watt)) {
    return "watt";
  } else {
    []<bool flag = false>() {
      static_assert(flag, "Missing dimension name, please add it to the list.");
    }
    ();
  }
}
}  // namespace tfc::unit
