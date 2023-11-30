#pragma once

#include <concepts>
#include <cstdint>
#include <string>

#include <mp-units/systems/si/si.h>

#include <tfc/ipc/enums.hpp>

namespace tfc::ipc::details {

namespace concepts {
template <typename given_t, typename... supposed_t>
concept is_any_of = (std::same_as<given_t, supposed_t> || ...);
template <typename given_t>
concept is_supported_type = is_any_of<given_t, bool, std::int64_t, std::uint64_t, double, std::string> || mp_units::Quantity<given_t>;
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
using type_mass = type_description<mp_units::quantity<mp_units::si::milli<mp_units::si::gram>, std::uint64_t>, type_e::_mass>;

}  // namespace tfc::ipc::details
