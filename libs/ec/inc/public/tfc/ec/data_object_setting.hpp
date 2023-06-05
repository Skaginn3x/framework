#pragma once

#include <concepts>

#include <tfc/confman/read_only.hpp>
#include <tfc/stx/basic_fixed_string.hpp>

#include "soem_interface.hpp"

// https://dataprotocols.org/units/
// https://github.com/frictionlessdata/specs/issues/216
// https://dataprotocols.org/json-table-schema/#field-types-and-formats

namespace tfc::ec {

/// \brief Generic setting struct for enums and arithmetic types
/// Please note that below are specialization of this same struct for useful unit notations like std::chrono
/// \tparam idx index of { index, subindex }
/// \tparam name the given parameter name by the vendor
/// \tparam desc human readable form of name by the vendor and or more information
/// \tparam value_t the representation of the setting value, remember to match the size by the vendor setting
/// \tparam value_t_construct_params forwarded argument pack sent to construct value_t for default value
template <ecx::index_t idx,
          tfc::stx::basic_fixed_string name,
          tfc::stx::basic_fixed_string desc,
          typename value_t,
          auto... value_t_construct_params>
struct setting {
  using type = value_t;
  static auto constexpr index{ idx };
  static std::string_view constexpr name_v{ name };
  static std::string_view constexpr desc_v{ desc };
  type value{ std::forward<decltype(value_t_construct_params)>(value_t_construct_params)... };
  auto operator==(setting const&) const noexcept -> bool = default;
  auto operator<=>(setting const&) const noexcept -> bool = default;
};
// So the size of setting is equal to the value size, meaning it can be reinterpreted.
static_assert(sizeof(setting<ecx::index_t{ 0x42, 0x42 }, "foo", "bar", uint32_t, 32>) == sizeof(uint32_t));

// std::chrono setting
// template <typename value_t>
// concept setting_chrono = requires {
//   typename value_t::rep;
//   typename value_t::period;
// };
// template <ecx::index_t idx,
//           tfc::stx::basic_fixed_string name,
//           tfc::stx::basic_fixed_string desc,
//           setting_chrono value_t,
//           auto... value_t_construct_params>
// struct setting<idx, name, desc, value_t, value_t_construct_params...> {
//   using type = value_t;
//   using rep_type = value_t::rep;
//   using period_type = typename value_t::period;
//   static auto constexpr index{ idx };
//   static std::string_view constexpr name_v{ name };
//   static std::string_view constexpr desc_v{ desc };
//   type value{ std::forward<decltype(value_t_construct_params)>(value_t_construct_params)... };
// };

// mp_units setting
template <typename value_t>
concept setting_units = requires {
                          value_t::reference;
                          value_t::quantity_spec;
                          value_t::dimension;
                          value_t::unit;
                          typename value_t::rep;
                        };
template <ecx::index_t idx,
          tfc::stx::basic_fixed_string name,
          tfc::stx::basic_fixed_string desc,
          setting_units value_t,
          auto... value_t_construct_params>
struct setting<idx, name, desc, value_t, value_t_construct_params...> {
  using type = value_t;
  using rep_type = typename value_t::rep;
  static auto constexpr index{ idx };
  static std::string_view constexpr name_v{ name };
  static std::string_view constexpr desc_v{ desc };
  type value{ std::forward<decltype(value_t_construct_params)>(value_t_construct_params)... };
};

namespace detail {

// TODO: when glaze adds support for constexpr this should be removed.
template <typename value_t>
struct setting_reader {
  using type = value_t;
  ecx::index_t index{};
  std::string_view name_v{};
  std::string_view desc_v{};
  type value{};
};
}  // namespace detail

}  // namespace tfc::ec

namespace glz {

template <typename value_t>
struct meta<tfc::ec::detail::setting_reader<value_t>> {
  using setting = tfc::ec::detail::setting_reader<value_t>;
  static constexpr auto value{
    object("value", &setting::value, "index", &setting::index, "name", &setting::name_v, "desc", &setting::desc_v)
  };
  static constexpr std::string_view name = "setting";
};

template <ecx::index_t idx,
          tfc::stx::basic_fixed_string name_value,
          tfc::stx::basic_fixed_string desc,
          typename value_t,
          auto... value_t_construct_params>
struct meta<tfc::ec::setting<idx, name_value, desc, value_t, value_t_construct_params...>> {
  using setting = tfc::ec::setting<idx, name_value, desc, value_t, value_t_construct_params...>;
  static auto constexpr value{ object("value", &setting::value) };
  static auto constexpr name{ "setting" };
};

// TODO: when glaze adds support for constexpr this should be removed.
namespace detail {
template <typename value_t>
struct to_json;

template <ecx::index_t idx,
          tfc::stx::basic_fixed_string name_value,
          tfc::stx::basic_fixed_string desc,
          typename value_t,
          value_t default_v>
struct to_json<tfc::ec::setting<idx, name_value, desc, value_t, default_v>> {
  template <auto Opts>
  static void op(auto& value, auto&&... args) noexcept {
    tfc::ec::detail::setting_reader<value_t> rep{
      .index = value.index, .name_v = value.name_v, .desc_v = value.desc_v, .value = value.value
    };
    write<json>::op<Opts>(rep, args...);
  }
};

}  // namespace glz
