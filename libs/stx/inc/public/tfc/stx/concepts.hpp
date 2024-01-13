#pragma once

#include <concepts>
#include <expected>
#include <system_error>
#include <type_traits>

// todo should we split to concepts/mp-units.hpp and concepts/stx.hpp
#include <mp-units/systems/si/si.h>

namespace tfc::stx {

template <typename given_t, typename... supposed_t>
concept is_any_of = (std::same_as<given_t, supposed_t> || ...);

template <typename func, typename... args>
concept nothrow_invocable = requires { std::is_nothrow_invocable_v<func, args...>; };

template <typename func, typename... args>
concept invocable = std::invocable<std::remove_cvref_t<func>, args...>;

template <typename enum_t>
concept is_enum = requires { requires std::is_enum_v<enum_t>; };

template <typename>
struct is_expected_impl : std::false_type {};
template <typename type, typename err>
struct is_expected_impl<std::expected<type, err>> : std::true_type {};
template <typename some_t>
concept is_expected = is_expected_impl<some_t>::value;

template <typename>
struct is_expected_quantity_impl : std::false_type {};
template <mp_units::Quantity type, typename err>
struct is_expected_quantity_impl<std::expected<type, err>> : std::true_type {};
template <typename some_t>
concept is_expected_quantity = is_expected_quantity_impl<some_t>::value;

template <typename compare_to_t, typename... args_t>
concept is_one_of = requires { requires(std::is_same_v<compare_to_t, args_t> || ...); };

// from
// https://stackoverflow.com/questions/16337610/how-to-know-if-a-type-is-a-specialization-of-stdvector
template <class, template <class...> class>
inline constexpr bool is_specialization_v = false;

template <template <class...> class type, class... args>
inline constexpr bool is_specialization_v<type<args...>, type> = true;

template <typename clock_t>
concept steady_clock = requires
{
  requires clock_t::is_steady;
  requires clock_t::is_steady == true;
} ;

namespace test {

struct empty {};
struct not_steady_clock {
  static constexpr auto is_steady{ false };
};
struct is_steady_clock {
  static constexpr auto is_steady{ true };
};
static_assert(!steady_clock<not_steady_clock>);
static_assert(!steady_clock<empty>);
static_assert(steady_clock<is_steady_clock>);

static_assert(is_expected_quantity<
              std::expected<mp_units::quantity<mp_units::si::milli<mp_units::si::gram>, std::uint64_t>, std::error_code>>);
static_assert(!is_expected_quantity<std::expected<std::uint64_t, std::error_code>>);
static_assert(!is_expected_quantity<std::uint16_t>);

}  // namespace test

}  // namespace tfc::stx
