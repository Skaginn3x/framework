#pragma once

#include <concepts>
#include <cstdint>
#include <expected>
#include <string>

#include <mp-units/systems/si/si.h>

#include <tfc/ipc/enums.hpp>
#include <tfc/stx/concepts.hpp>

namespace tfc::ipc::details {

namespace concepts {
using stx::is_any_of;
using stx::is_expected_quantity;
template <typename given_t>
concept is_supported_type =
    is_any_of<given_t, bool, std::int64_t, std::uint64_t, double, std::string> || is_expected_quantity<given_t>;
}  // namespace concepts

template <concepts::is_supported_type value_type, type_e type_enum>
struct type_description {
  using value_t = value_type;
  static constexpr auto value_e = type_enum;
  static constexpr std::string_view type_name{ enum_name(type_enum) };
};

using type_bool = type_description<bool, type_e::_bool>;
using type_int = type_description<std::int64_t, type_e::_int64_t>;
using type_uint = type_description<std::uint64_t, type_e::_uint64_t>;
using type_double = type_description<double, type_e::_double_t>;
using type_string = type_description<std::string, type_e::_string>;
using type_json = type_description<std::string, type_e::_json>;
using mass_t = std::expected<mp_units::quantity<mp_units::si::milli<mp_units::si::gram>, std::int64_t>, mass_error_e>;
using type_mass = type_description<mass_t, type_e::_mass>;

}  // namespace tfc::ipc::details
