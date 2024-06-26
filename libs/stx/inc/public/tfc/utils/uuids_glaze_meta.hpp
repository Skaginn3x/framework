#pragma once

#include <stduuid/uuid.h>

namespace glz {

template <typename>
struct meta;

template <>
struct meta<uuids::uuid> {
  using type = uuids::uuid;
  static constexpr auto name{ "uuids::uuid" };
};

namespace detail {

template <>
struct from_json<uuids::uuid> {
  template <auto opts>
  inline static void op(auto& value, auto&&... args) noexcept {
    std::string id{};
    from_json<std::string>::template op<opts>(id, std::forward<decltype(args)>(args)...);
    auto parsed{ uuids::uuid::from_string(id) };
    if (parsed) {
      value = std::move(parsed.value());
    }
  }
};
template <>
struct to_json<uuids::uuid> {
  template <auto opts>
  static void op(auto& value, auto&&... args) noexcept {
    write<json>::op<opts>(uuids::to_string(value), args...);
  }
};

template <typename value_t>
struct to_json_schema;

template <>
struct to_json_schema<uuids::uuid> {
  template <auto opts>
  static void op(auto& schema, auto& defs) {
    // fix in https://github.com/Skaginn3x/framework/issues/555
    // using enum tfc::json::defined_formats;
    // schema.attributes.format = uuid;
    to_json_schema<std::string>::op<opts>(schema, defs);
  }
};

}  // namespace detail
}  // namespace glz
