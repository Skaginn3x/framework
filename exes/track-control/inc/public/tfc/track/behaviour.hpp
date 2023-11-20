#pragma once

#include <string>

#include <mp-units/systems/si/si.h>
#include <glaze/core/common.hpp>

#include <tfc/utils/json_schema.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace tfc::track {

namespace si = mp_units::si;

template <typename variant_t>
struct behaviour {
  std::string name{};
  mp_units::quantity<si::milli<si::metre>, std::uint64_t> loc{};
  variant_t behaviour{};
};

}  // namespace tfc::track

namespace glz {

template <typename>
struct meta;

template <typename variant_t>
struct meta<tfc::track::behaviour<variant_t>> {
  using type = tfc::track::behaviour<variant_t>;
  static constexpr std::string_view name{ "tfc::track::location" };
  // clang-format off
  static constexpr auto value{ glz::object(
    "name", &type::name, tfc::json::schema{ .description = "The human readable name of the location", .pattern = "^[a-zA-Z0-9.]*$"},
    "loc", &type::loc, tfc::json::schema{ .description = "Location of the given behaviour" }, // todo maximum length should be length of the track
    "behaviour", &type::behaviour, tfc::json::schema{ .description = "The behaviour of the location" }
    )
  };
  // clang-format on
};

}  // namespace glz
