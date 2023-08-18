#pragma once

#include <cstdint>
#include <string_view>

#include <units/isq/si/si.h>
#include <units/magnitude.h>
#include <units/quantity.h>
#include <units/ratio.h>
#include <glaze/glaze.hpp>

#include <tfc/stx/string_view_join.hpp>

namespace tfc::unit {
template <typename dim_t>
inline constexpr auto dimension_name() -> std::string_view;
}  // namespace tfc::unit

template <>
struct glz::meta<units::ratio> {
  static constexpr auto value{ glz::object("numerator", &units::ratio::num, "denominator", &units::ratio::den) };
  static constexpr auto name{ "units::ratio" };
};

template <typename dimension_t, typename unit_t, typename rep_t>
struct glz::meta<units::quantity<dimension_t, unit_t, rep_t>> {
  using type = units::quantity<dimension_t, unit_t, rep_t>;
  static constexpr std::string_view unit{ unit_t::symbol.standard().data_ };
  static constexpr auto dimension{ tfc::unit::dimension_name<dimension_t>() };
  static auto constexpr value{ [](auto&& self) -> auto& { return self.number();
}
}
;
static std::string_view constexpr prefix{ "units::quantity<" };
static std::string_view constexpr postfix{ ">" };
static std::string_view constexpr separator{ "," };
static auto constexpr name{
  tfc::stx::string_view_join_v<prefix, dimension, separator, unit, separator, glz::name_v<rep_t>, postfix>
};
}
;

namespace tfc::json::detail {

template <typename value_t>
struct to_json_schema;

template <typename dimension_t, typename unit_t, typename rep_t>
struct to_json_schema<units::quantity<dimension_t, unit_t, rep_t>> {
  static constexpr std::string_view unit{ unit_t::symbol.standard().data_ };
  static constexpr units::ratio ratio{ units::as_ratio(unit_t::mag) };
  static constexpr auto dimension{ tfc::unit::dimension_name<dimension_t>() };
  template <auto opts>
  static void op(auto& schema, auto& defs) {
    auto& data = schema.attributes.tfc_metadata;
    if (!data.has_value()) {
      data = tfc::json::schema_meta{};
    }
    data->unit = unit;
    data->dimension = dimension;
    data->ratio = tfc::json::schema_meta::ratio_impl{ .numerator = ratio.num, .denominator = ratio.den };
    to_json_schema<rep_t>::template op<opts>(schema, defs);
  }
};

}  // namespace tfc::json::detail

namespace tfc::unit {
template <typename dim_t>
inline constexpr auto dimension_name() -> std::string_view {
  using stripped_dim_t = std::remove_cvref_t<dim_t>;
  namespace si = units::isq::si;
  if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_length>) {
    return "length";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_time>) {
    return "time";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_area>) {
    return "area";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_volume>) {
    return "volume";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_speed>) {
    return "speed";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_angular_velocity>) {
    return "angular_velocity";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_acceleration>) {
    return "acceleration";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_angular_acceleration>) {
    return "angular_acceleration";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_capacitance>) {
    return "capacitance";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_conductance>) {
    return "conductance";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_energy>) {
    return "energy";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_force>) {
    return "force";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_frequency>) {
    return "frequency";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_heat_capacity>) {
    return "heat_capacity";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_mass>) {
    return "mass";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_voltage>) {
    return "voltage";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_electric_current>) {
    return "electric_current";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_inductance>) {
    return "inductance";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_power>) {
    return "power";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_resistance>) {
    return "resistance";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_pressure>) {
    return "pressure";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_torque>) {
    return "torque";
  } else if constexpr (std::is_convertible_v<stripped_dim_t, si::dim_luminance>) {
    return "luminance";
  } else {
    []<bool flag = false>() {
      static_assert(flag, "Missing dimension name, please add it to the list. Or use compile time regex.");
    }
    ();
  }
}
}  // namespace tfc::unit
