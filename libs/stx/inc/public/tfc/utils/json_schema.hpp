// copy from 83d1cb0e924d1adeb6c9a18d04ca0b7fe95e44f4
// Glaze Library
// For the license information refer to glaze.hpp
#pragma once

#include <algorithm>

#include <glaze/api/impl.hpp>
#include <glaze/core/common.hpp>
#include <glaze/core/meta.hpp>
#include <glaze/json/write.hpp>
#include <glaze/util/for_each.hpp>

namespace tfc::json {

using schema = glz::schema;


namespace detail {
//
// template <typename value_t>
// struct to_json_schema {
//   template <auto Opts>
//   static void op(auto& schema, auto& defs) noexcept
//   {
//     glz::detail::to_json_schema<value_t>::op<>()
//   }
// };

}  // namespace detail

template <class T, glz::opts Opts = glz::opts{}>
inline auto write_json_schema() noexcept {
  return glz::write_json_schema<T, Opts>();
}
}  // namespace tfc::json
