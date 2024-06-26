#pragma once

#include <concepts>
#include <expected>
#include <optional>
#include <system_error>
#include <type_traits>

// todo should we split to concepts/mp-units.hpp and concepts/stx.hpp
#include <mp-units/systems/si.h>

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
concept steady_clock = requires {
  requires clock_t::is_steady;
  requires clock_t::is_steady == true;
};

template <typename T>
concept is_optional = requires(T t) {
  typename T::value_type;
  requires std::same_as<T, std::optional<typename T::value_type>>;
};

// From https://en.cppreference.com/w/cpp/experimental/is_detected
namespace detail {
template <class Default, class AlwaysVoid, template <class...> class Op, class... Args>
struct detector {
  using value_t = std::false_type;
  using type = Default;
};

template <class Default, template <class...> class Op, class... Args>
struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
  using value_t = std::true_type;
  using type = Op<Args...>;
};
struct nonesuch {
  ~nonesuch() = delete;
  nonesuch(nonesuch const&) = delete;
  void operator=(nonesuch const&) = delete;
};

}  // namespace detail

template <template <class...> class Op, class... Args>
using is_detected = typename detail::detector<detail::nonesuch, void, Op, Args...>::value_t;

template <template <class...> class Op, class... Args>
constexpr bool is_detected_v = is_detected<Op, Args...>::value;

template <template <class...> class Op, class... Args>
using detected_t = typename detail::detector<detail::nonesuch, void, Op, Args...>::type;

template <class Default, template <class...> class Op, class... Args>
using detected_or = detail::detector<Default, void, Op, Args...>;
// endof From https://en.cppreference.com/w/cpp/experimental/is_detected

namespace detail {
template <typename T, typename = void>
struct is_constexpr_default_constructible : std::false_type {};

template <typename T>
struct is_constexpr_default_constructible<T, std::enable_if_t<std::is_default_constructible_v<T>>>
    : std::bool_constant<noexcept(T())> {
private:
  static constexpr bool check() {
    [[maybe_unused]] constexpr T t{};
    return true;
  }

public:
  static constexpr bool value = check();
};
}  // namespace detail
template <typename T>
inline constexpr bool is_constexpr_default_constructible_v = detail::is_constexpr_default_constructible<T>::value;

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

static_assert(is_optional<std::optional<int>>);
static_assert(!is_optional<std::vector<int>>);
static_assert(!is_optional<int>);

static_assert(is_expected_quantity<
              std::expected<mp_units::quantity<mp_units::si::milli<mp_units::si::gram>, std::uint64_t>, std::error_code>>);
static_assert(!is_expected_quantity<std::expected<std::uint64_t, std::error_code>>);
static_assert(!is_expected_quantity<std::uint16_t>);

}  // namespace test

}  // namespace tfc::stx
