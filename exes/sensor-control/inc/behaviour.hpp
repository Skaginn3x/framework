#pragma once

#include <string>

#include <mp-units/systems/si/si.h>
#include <glaze/core/common.hpp>

#include <tfc/utils/json_schema.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace tfc::sensor::control {

namespace si = mp_units::si;

template <typename variant_t>
struct behaviour {
  using type = variant_t::type;
  std::string name{};
  mp_units::quantity<si::milli<si::metre>, std::uint64_t> loc{};
  variant_t behave{};
};

}  // namespace tfc::track

namespace glz {

template <typename>
struct meta;

template <typename variant_t>
struct meta<tfc::sensor::control::behaviour<variant_t>> {
  using type = tfc::sensor::control::behaviour<variant_t>;
  static constexpr std::string_view name{ "tfc::sensor::control::location" };
  // clang-format off
  static constexpr auto value{ glz::object(
    "name", &type::name, tfc::json::schema{ .description = "The human readable name of the location", .pattern = "^[a-zA-Z0-9.]*$"},
    "loc", &type::loc, tfc::json::schema{ .description = "Location of the given behaviour" }, // todo maximum length should be length of the track
    "behave", &type::behave, tfc::json::schema{ .description = "The behaviour of the location" }
    )
  };
  // clang-format on
};

}  // namespace glz
