#pragma once

#include <concepts>

#include <glaze/glaze.hpp>

#include <tfc/ec/soem_interface.hpp>
#include <tfc/stx/basic_fixed_string.hpp>

namespace tfc::ec::util {
template <typename from, typename to>
auto map(from value, from in_min, from in_max, to out_min, to out_max) -> to {
  return static_cast<to>((static_cast<double>(value - in_min)) * static_cast<double>(out_max - out_min) /
                             static_cast<double>(in_max - in_min) +
                         static_cast<double>(out_min));
}

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

}  // namespace tfc::ec::util

template <ecx::index_t idx,
          tfc::stx::basic_fixed_string name_value,
          tfc::stx::basic_fixed_string desc,
          typename value_t,
          auto... value_t_construct_params>
struct glz::meta<tfc::ec::util::setting<idx, name_value, desc, value_t, value_t_construct_params...>> {
  using setting = tfc::ec::util::setting<idx, name_value, desc, value_t, value_t_construct_params...>;
  static constexpr auto value{
    glz::object("value", &setting::value, "index", &setting::index, "name", &setting::name_v, "desc", &setting::desc_v)
  };
  static constexpr std::string_view name = "tfc::ec::setting";
};
