#pragma once

#include <string_view>
#include <type_traits>
#include <concepts>

#include <boost/sml.hpp>

#include <tfc/stx/concepts.hpp>


namespace tfc::sml::concepts {

template <typename type_t>
concept name_exists = requires {
  { type_t::name };
  requires std::same_as<std::string_view, std::remove_cvref_t<decltype(type_t::name)>>;
};

template <typename T>
concept is_sub_sm = tfc::stx::is_specialization_v<T, boost::sml::back::sm>;


namespace test {
struct test_state {
  static constexpr std::string_view name{ "test_state" };
};
struct invalid_test_state {
  static constexpr std::string_view name1{ "test_state" };
};
static_assert(name_exists<test_state>);
static_assert(!name_exists<invalid_test_state>);
}  // namespace test

}
