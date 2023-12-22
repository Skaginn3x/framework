#pragma once

#include <glaze/core/common.hpp>
#include <glaze/core/meta.hpp>
#include <string_view>

#include <mp-units/bits/ratio.h>
#include <mp-units/quantity.h>
#include <mp-units/systems/angular/angular.h>
#include <mp-units/systems/si/units.h>
#include <mp-units/unit.h>

#include <tfc/stx/string_view_join.hpp>
#include <tfc/utils/json_schema.hpp>

namespace tfc::unit {
template <mp_units::Reference auto ref_t>
consteval auto dimension_name() -> std::string_view;
}  // namespace tfc::unit

template <>
struct glz::meta<mp_units::ratio> {
  static constexpr auto value{ glz::object("numerator", &mp_units::ratio::num, "denominator", &mp_units::ratio::den) };
  static constexpr auto name{ "units::ratio" };
};

template <mp_units::Reference auto ref_t, typename rep_t>
struct glz::meta<mp_units::quantity<ref_t, rep_t>> {
  using type = mp_units::quantity<ref_t, rep_t>;
  static constexpr std::string_view unit{
    mp_units::unit_symbol<mp_units::unit_symbol_formatting{ .encoding = mp_units::text_encoding::ascii }>(ref_t)
  };
  static constexpr auto dimension{ tfc::unit::dimension_name<ref_t>() };
  static auto constexpr value{ [](auto&& self) -> auto& { return self.numerical_value_ref_in(type::unit); } };
  static std::string_view constexpr prefix{ "units::quantity<" };
  static std::string_view constexpr postfix{ ">" };
  static std::string_view constexpr separator{ "," };
  static auto constexpr name{
    tfc::stx::string_view_join_v<prefix, dimension, separator, unit, separator, glz::name_v<rep_t>, postfix>
  };
};

namespace tfc::json::detail {

template <typename value_t>
struct to_json_schema;

template <mp_units::Reference auto ref_t, typename rep_t>
struct to_json_schema<mp_units::quantity<ref_t, rep_t>> {
  static constexpr std::string_view unit_ascii{
    mp_units::unit_symbol<mp_units::unit_symbol_formatting{ .encoding = mp_units::text_encoding::ascii }>(ref_t)
  };
  static constexpr std::string_view unit_unicode{
    mp_units::unit_symbol<mp_units::unit_symbol_formatting{ .encoding = mp_units::text_encoding::unicode }>(ref_t)
  };
  static constexpr mp_units::ratio ratio{ mp_units::as_ratio(ref_t) };
  static constexpr auto dimension{ tfc::unit::dimension_name<ref_t>() };
  template <auto opts>
  static void op(auto& schema, auto& defs) {
    auto& data = schema.attributes.tfc_metadata;
    if (!data.has_value()) {
      data = tfc::json::schema_meta{};
    }
    data->unit = schema_meta::unit_meta{ .unit_ascii = "", .unit_unicode = "" };
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
// based of https://github.com/gentooboontoo/js-quantities/blob/master/src/quantities/kind.js#L5
inline consteval auto dimension_name() -> std::string_view {
  // todo: the following
  // "elastance",
  // "resistance",
  // "inductance",
  // "magnetism",
  // "magnetism",
  // "specific_volume",
  // "snap",
  // "jolt",
  // "radiation",
  // "viscosity",
  // "volumetric_flow",
  // "wavenumber",
  // "unitless",
  // "time",
  // "temperature",
  // "yank",
  // "pressure",
  // "force",
  // "energy",
  // "viscosity",
  // "momentum",
  // "angular_momentum",
  // "density",
  // "area_density",
  // "radiation_exposure",
  // "magnetism",
  // "charge",
  // "conductance",
  // "capacitance",
  // "activity",
  // "molar_concentration",
  // "substance",
  // "illuminance",
  // "luminous_power",
  // "currency",
  // "information_rate",
  // "information",
  // "angular_velocity",

  if constexpr (mp_units::convertible(ref_t, mp_units::si::metre)) {
    return "length";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::hertz)) {
    return "frequency";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::ampere)) {
    return "current";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::volt)) {
    return "potential";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::watt)) {
    return "power";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::gram)) {
    return "mass";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::litre)) {
    return "volume";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::angular::degree)) {
    return "angle";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::square(mp_units::si::metre))) {
    return "area";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::metre / mp_units::si::second)) {
    return "speed";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::metre / mp_units::square(mp_units::si::second))) {
    return "acceleration";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::percent)) {
    return "ratio";
  } else {
    []<bool flag = false>() {
      static_assert(flag, "Missing dimension name, please add it to the list.");
    }
    ();
  }
}
}  // namespace tfc::unit
