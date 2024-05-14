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

// The implementation of this inside mp-units has some issues with clang 17
// At some point in the future it might be good to try and remove this
// And see if it is resolved. The issue was that the returned string
// would consist of null terminations only for the entire length
// of the string. Unable to replicate inside of mp-units using clang 17
// but it is a problem in this project. The method to use inside of mp-units
// is unit_symbol.
// MIX BEGIN
template <typename CharT, mp_units::Unit U, mp_units::unit_symbol_formatting fmt>
struct const_unit_symbol {
  static consteval auto unit_symbol_len() -> std::size_t { return unit_symbol<fmt, CharT>(U{}).size(); }
  static constexpr auto impl() noexcept {
    std::array<CharT, unit_symbol_len() + 1> buffer{};
    auto foo = mp_units::detail::get_symbol_buffer<CharT, unit_symbol_len(), fmt>(U{});
    std::ranges::copy(std::begin(foo), std::end(foo), std::begin(buffer));
    return buffer;
  }
  // Give the joined string static storage
  static constexpr auto arr = impl();
  // View as a std::string_view
  static constexpr std::basic_string_view<CharT> value{ arr.data(), arr.size() - 1 };
};

template <mp_units::unit_symbol_formatting fmt = mp_units::unit_symbol_formatting{}, typename CharT = char, mp_units::Unit U>
[[nodiscard]] constexpr std::basic_string_view<CharT> unit_symbol_view(U) {
  return const_unit_symbol<CharT, U, fmt>::value;
}
// MIX END

template <mp_units::Reference auto ref_t, typename rep_t>
struct glz::meta<mp_units::quantity<ref_t, rep_t>> {
  using type = mp_units::quantity<ref_t, rep_t>;
  static constexpr std::string_view unit{
    unit_symbol_view<mp_units::unit_symbol_formatting{ .encoding = mp_units::text_encoding::ascii }>(ref_t)
  };
  static constexpr auto dimension{ tfc::unit::dimension_name<ref_t>() };
  static auto constexpr value{ [](auto&& self) -> auto& { return self.numerical_value_ref_in(type::unit); } };
  static std::string_view constexpr prefix{ "units::quantity<" };
  static std::string_view constexpr postfix{ ">" };
  static std::string_view constexpr separator{ "," };
  static auto constexpr name{ tfc::stx::string_view_join_v<prefix, dimension, separator, glz::name_v<rep_t>, postfix> };
};

namespace glz::detail {

template <typename value_t>
struct to_json_schema;

template <mp_units::Reference auto ref_t, typename rep_t>
struct to_json_schema<mp_units::quantity<ref_t, rep_t>> {
  static constexpr std::string_view unit_ascii{
    unit_symbol_view<mp_units::unit_symbol_formatting{ .encoding = mp_units::text_encoding::ascii }>(ref_t)
  };
  static constexpr std::string_view unit_unicode{
    unit_symbol_view<mp_units::unit_symbol_formatting{ .encoding = mp_units::text_encoding::unicode }>(ref_t)
  };
  static_assert(unit_ascii.size() > 0);
  static_assert(unit_unicode.size() > 0);
  static_assert(unit_ascii[0] != 0);
  static_assert(unit_unicode[0] != 0);

  static constexpr mp_units::ratio ratio{ mp_units::as_ratio(ref_t) };
  static constexpr auto dimension{ tfc::unit::dimension_name<ref_t>() };
  template <auto opts>
  static void op([[maybe_unused]] auto& schema, [[maybe_unused]] auto& defs) {
    // fix in https://github.com/Skaginn3x/framework/issues/555
    // auto& data = schema.attributes.tfc_metadata;
    // if (!data.has_value()) {
    //   data = tfc::json::schema_meta{};
    // }
    // data->unit = schema_meta::unit_meta{ .unit_ascii = unit_ascii, .unit_unicode = unit_unicode };
    // data->dimension = dimension;
    // if constexpr (mp_units::Magnitude<decltype(ref_t)>) {
    //   data->ratio = tfc::json::schema_meta::ratio_impl{ .numerator = ratio.num, .denominator = ratio.den };
    // }
    // to_json_schema<rep_t>::template op<opts>(schema, defs);
  }
};

}  // namespace glz::detail

namespace tfc::unit {
template <mp_units::Reference auto ref_t>
// based of https://github.com/gentooboontoo/js-quantities/blob/master/src/quantities/kind.js#L5
inline consteval auto dimension_name() -> std::string_view {
  // todo: the following
  // "elastance",
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
  // "yank",
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
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::second)) {
    return "time";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::henry)) {
    return "inductance";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::ohm)) {
    return "resistance";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::pascal)) {
    return "pressure";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::kelvin)) {
    return "temperature";
  } else if constexpr (mp_units::convertible(ref_t, mp_units::si::litre / mp_units::si::second)) {
    return "flow";
  } else {
    []<bool flag = false>() {
      static_assert(flag, "Missing dimension name, please add it to the list.");
    }
    ();
  }
}
}  // namespace tfc::unit
