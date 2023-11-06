#pragma once

#include <concepts>

#include <glaze/glaze.hpp>

#include <tfc/ec/soem_interface.hpp>
#include <tfc/stx/basic_fixed_string.hpp>
#include <tfc/stx/string_view_join.hpp>
#include <tfc/stx/to_string_view.hpp>
#include <tfc/utils/json_schema.hpp>

namespace tfc::ec::util {
template <typename from, typename to>
inline constexpr auto map(from value, from in_min, from in_max, to out_min, to out_max) noexcept -> to {
  return static_cast<to>(static_cast<double>(std::max(std::min(value, in_max), in_min) - in_min) *
                             static_cast<double>(out_max - out_min) / static_cast<double>(in_max - in_min) +
                         static_cast<double>(out_min));
}
static_assert(map(10, 0, 10, 0, 20) == 20);
static_assert(map(100000, 0, 10, 0, 20) == 20);  // above max
static_assert(map(0, 1, 10, 0, 20) == 0);        // below min
static_assert(map(500, 0, 1000, 0, 20) == 10);
static_assert(map(500.0, 0.0, 1000.0, 0, 20) == 10);
static_assert(map(500, 0, 1000, 0.0, 20.0) == 10);

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
  type value{ value_t_construct_params... };

  auto operator==(setting const&) const noexcept -> bool = default;
  auto operator<=>(setting const&) const noexcept -> bool = default;
};
// So the size of setting is equal to the value size, meaning it can be reinterpreted.
static_assert(sizeof(setting<ecx::index_t{ 0x42, 0x42 }, "foo", "bar", uint32_t, 32>) == sizeof(uint32_t));

}  // namespace tfc::ec::util

template <ecx::index_t idx,
          tfc::stx::basic_fixed_string name_value,
          tfc::stx::basic_fixed_string desc,
          typename value_t,
          auto... value_t_construct_params>
struct glz::meta<tfc::ec::util::setting<idx, name_value, desc, value_t, value_t_construct_params...>> {
  using setting = tfc::ec::util::setting<idx, name_value, desc, value_t, value_t_construct_params...>;
  static constexpr auto value{ &setting::value };
  static constexpr std::string_view name_prefix = "tfc::ec::setting::";
  static constexpr std::string_view name_suffix = name_value;
  static constexpr std::string_view name = tfc::stx::string_view_join_v<name_prefix, name_suffix>;
};
namespace tfc::json::detail {

template <ecx::index_t idx,
          tfc::stx::basic_fixed_string name_value,
          tfc::stx::basic_fixed_string desc,
          typename value_t,
          auto... value_t_construct_params>
struct to_json_schema<tfc::ec::util::setting<idx, name_value, desc, value_t, value_t_construct_params...>> {
  using setting = tfc::ec::util::setting<idx, name_value, desc, value_t, value_t_construct_params...>;
  static constexpr auto name_view{ name_value.view() };
  static constexpr auto description{ tfc::stx::string_view_join_v<   //
      glz::chars<" variable(">,                                      //
      name_view,                                                     //
      glz::chars<") at index 0x">,                                   //
      tfc::stx::to_string_view_v<idx.first, tfc::stx::base_e::hex>,  //
      glz::chars<" sub 0x">,                                         //
      tfc::stx::to_string_view_v<idx.second, tfc::stx::base_e::hex>  //
      > };
  template <auto opts>
  static void op(auto& schema, auto& defs) {
    schema.attributes.title = desc;
    schema.attributes.description = description;
    to_json_schema<value_t>::template op<opts>(schema, defs);
  }
};

}  // namespace tfc::json::detail
