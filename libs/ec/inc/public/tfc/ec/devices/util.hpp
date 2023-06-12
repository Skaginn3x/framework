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

template <ecx::index_t idx,
          tfc::stx::basic_fixed_string name,
          tfc::stx::basic_fixed_string desc,
          typename value_t,
          value_t default_v>
struct setting {
  using type = value_t;
  static auto constexpr index{ idx };
  static std::string_view constexpr name_v{ name };
  static std::string_view constexpr desc_v{ desc };
  type value{ default_v };

  auto operator==(setting const&) const noexcept -> bool = default;
};

// So the size of setting is equal to the value size, meaning it can be reinterpreted.
static_assert(sizeof(setting<ecx::index_t{ 0x42, 0x42 }, "foo", "bar", uint32_t, 32>) == sizeof(uint32_t));

template <typename value_t>
struct setting_reader {
  using type = value_t;
  ecx::index_t index{};
  std::string_view name_v{};
  std::string_view desc_v{};
  type value{};
};
}  // namespace tfc::ec::util

template <typename value_t>
struct glz::meta<tfc::ec::util::setting_reader<value_t>> {
  using setting = tfc::ec::util::setting_reader<value_t>;
  //  clang-format off
  static constexpr auto value{
    glz::object("value", &setting::value, "index", &setting::index, "name", &setting::name_v, "desc", &setting::desc_v)
  };
  // clang-format on
  static constexpr std::string_view name = "setting";
};
template <ecx::index_t idx,
          tfc::stx::basic_fixed_string name_value,
          tfc::stx::basic_fixed_string desc,
          typename value_t,
          value_t default_v>
struct glz::meta<tfc::ec::util::setting<idx, name_value, desc, value_t, default_v>> {
  using setting = tfc::ec::util::setting<idx, name_value, desc, value_t, default_v>;
  //  clang-format off
  static constexpr auto value{ glz::object("value", &setting::value) };
  // clang-format on
  static constexpr std::string_view name = "setting";
};
// TODO: when glaze adds support for constexpr this should be removed.
namespace glz::detail {
template <ecx::index_t idx,
          tfc::stx::basic_fixed_string name_value,
          tfc::stx::basic_fixed_string desc,
          typename value_t,
          value_t default_v>
struct to_json<tfc::ec::util::setting<idx, name_value, desc, value_t, default_v>> {
  template <auto Opts>
  static void op(auto& value, auto&&... args) noexcept {
    tfc::ec::util::setting_reader<value_t> rep{
      .index = value.index, .name_v = value.name_v, .desc_v = value.desc_v, .value = value.value
    };
    write<json>::op<Opts>(rep, args...);
  }
};
}  // namespace glz::detail