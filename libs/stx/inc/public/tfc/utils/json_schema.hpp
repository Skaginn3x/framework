// copy from 83d1cb0e924d1adeb6c9a18d04ca0b7fe95e44f4
// Glaze Library
// For the license information refer to glaze.hpp
#pragma once

#include <glaze/api/impl.hpp>
#include <glaze/core/common.hpp>
#include <glaze/core/meta.hpp>
#include <glaze/json/write.hpp>
#include <glaze/util/for_each.hpp>

namespace tfc::json {
struct schema_meta final {
  struct ratio_impl {
    std::uint64_t numerator{};
    std::uint64_t denominator{};
    struct glaze {
      // clang-format off
      static constexpr auto value{ glz::object(
          "numerator", &ratio_impl::numerator,
          "denominator", &ratio_impl::denominator
          ) };
      // clang-format on
    };
  };
  struct unit_meta final {
    std::string_view unit_ascii{};
    std::string_view unit_unicode{};
    struct glaze {
      // clang-format off
      static constexpr auto value{ glz::object(
        "unit_ascii", &unit_meta::unit_ascii,
        "unit_unicode", &unit_meta::unit_unicode
      ) };
      // clang-format on
    };
  };
  bool required{ true };
  std::optional<unit_meta> unit{};
  std::optional<std::string_view> dimension{};
  std::optional<ratio_impl> ratio{};
  struct glaze {
    using type = schema_meta;
    // clang-format off
    static constexpr auto value{ glz::object(
        "unit", &type::unit,
        "dimension", &type::dimension,
        "ratio", &type::ratio,
        "required", &type::required
        ) };
    // clang-format on
  };
};

enum struct defined_formats : std::uint8_t {
  datetime,
  date,
  time,
  duration,
  email,
  idn_email,
  hostname,
  idn_hostname,
  ipv4,
  ipv6,
  uri,
  uri_reference,
  iri,
  iri_reference,
  uuid,
  uri_template,
  json_pointer,
  relative_json_pointer,
  regex
};

struct schema {
  std::string_view ref{};
  using schema_number = std::optional<std::variant<std::int64_t, std::uint64_t, double>>;
  using schema_any = std::variant<std::monostate, bool, std::int64_t, std::uint64_t, double, std::string_view>;
  // meta data keywords, ref: https://www.learnjsonschema.com/2020-12/meta-data/
  std::optional<std::string_view> title{};
  std::optional<std::string_view> description{};
  std::optional<schema_any> default_value{};
  std::optional<bool> deprecated{};
  // std vector requires exit destructor & global constructor
  //  std::optional<std::vector<schema_any>> examples{};
  std::optional<bool> read_only{};
  std::optional<bool> write_only{};
  // hereafter validation keywords, ref: https://www.learnjsonschema.com/2020-12/validation/
  std::optional<schema_any> constant{};
  // string only keywords
  std::optional<std::uint64_t> min_length{};
  std::optional<std::uint64_t> max_length{};
  std::optional<std::string_view> pattern{};
  // https://www.learnjsonschema.com/2020-12/format-annotation/format/
  std::optional<defined_formats> format{};
  // number only keywords
  schema_number minimum{};
  schema_number maximum{};
  schema_number exclusive_minimum{};
  schema_number exclusive_maximum{};
  schema_number multiple_of{};
  // object only keywords
  std::optional<std::uint64_t> min_properties{};
  std::optional<std::uint64_t> max_properties{};
  //      std::optional<std::map<std::string_view, std::vector<std::string_view>>> dependent_required{};
  //  std::optional<std::vector<std::string_view>> required{};
  // array only keywords
  std::optional<std::uint64_t> min_items{};
  std::optional<std::uint64_t> max_items{};
  std::optional<std::uint64_t> min_contains{};
  std::optional<std::uint64_t> max_contains{};
  std::optional<bool> unique_items{};

  // metadata
  std::optional<schema_meta> tfc_metadata{};

  static constexpr auto schema_attributes{ true };  // allowance flag to indicate metadata within glz::Object

  // TODO switch to using variants when we have write support to get rid of nulls
  // TODO We should be able to generate the json schema compiletime
  struct glaze {
    using T = schema;
    // clang-format off
    static constexpr auto value = glz::object(
      "$ref", &T::ref, //
      "title", &T::title, //
      "description", &T::description, //
      "default", &T::default_value, //
      "deprecated", &T::deprecated, //
//      "examples", &T::examples, //
      "readOnly", &T::read_only, //
      "writeOnly", &T::write_only, //
      "const", &T::constant, //
      "minLength", &T::min_length, //
      "maxLength", &T::max_length, //
      "pattern", &T::pattern,
      "format", &T::format, //
      "minimum", &T::minimum, //
      "maximum", &T::maximum, //
      "exclusiveMinimum", &T::exclusive_minimum, //
      "exclusiveMaximum", &T::exclusive_maximum, //
      "multipleOf", &T::multiple_of, //
      "minProperties", &T::min_properties, //
      "maxProperties", &T::max_properties, //
//    "dependentRequired", &T::dependent_required, //
//      "required", &T::required, //
      "minItems", &T::min_items, //
      "maxItems", &T::max_items, //
      "minContains", &T::min_contains, //
      "maxContains", &T::max_contains, //
      "uniqueItems", &T::unique_items, //
      "x-tfc", &T::tfc_metadata
    );
    // clang-format on
  };
};

namespace detail {
struct schematic {
  std::optional<std::vector<std::string_view>> type{};
  std::optional<std::map<std::string_view, schema, std::less<>>> properties{};  // glaze_object
  std::optional<schema> items{};                                                // array
  std::optional<std::variant<bool, schema>> additionalProperties{};             // map
  std::optional<std::map<std::string_view, schematic, std::less<>>> defs{};
  std::optional<std::vector<std::string_view>> enumeration{};  // enum
  std::optional<std::vector<schematic>> oneOf{};
  schema attributes{};
};
}  // namespace detail
}  // namespace tfc::json

template <>
struct glz::meta<tfc::json::detail::schematic> {
  static constexpr std::string_view name = "glz::detail::schema";
  using T = tfc::json::detail::schematic;
  // clang-format off
  static constexpr auto value = glz::object(
      "type", &T::type,  //
      "properties", &T::properties,  //
      "items", &T::items,  //
      "additionalProperties", &T::additionalProperties,  //
      "$defs", &T::defs,  //
      "enum", &T::enumeration,  //
      "oneOf", &T::oneOf,  //
      "title", [](auto&& self) -> auto& { return self.attributes.title; }, //
      "description", [](auto&& self) -> auto& { return self.attributes.description; }, //
      "default", [](auto&& self) -> auto& { return self.attributes.default_value; }, //
      "deprecated", [](auto&& self) -> auto& { return self.attributes.deprecated; }, //
      //      "examples", [](auto&& self) -> auto& { return self.attributes.examples; }, //
      "readOnly", [](auto&& self) -> auto& { return self.attributes.read_only; }, //
      "writeOnly", [](auto&& self) -> auto& { return self.attributes.write_only; }, //
      "const", [](auto&& self) -> auto& { return self.attributes.constant; }, //
      "minLength", [](auto&& self) -> auto& { return self.attributes.min_length; }, //
      "maxLength", [](auto&& self) -> auto& { return self.attributes.max_length; }, //
      "pattern", [](auto&& self) -> auto& { return self.attributes.pattern; },
      "format", [](auto&& self) -> auto& { return self.attributes.format; },
      "minimum", [](auto&& self) -> auto& { return self.attributes.minimum; }, //
      "maximum", [](auto&& self) -> auto& { return self.attributes.maximum; }, //
      "exclusiveMinimum", [](auto&& self) -> auto& { return self.attributes.exclusive_minimum; }, //
      "exclusiveMaximum", [](auto&& self) -> auto& { return self.attributes.exclusive_maximum; }, //
      "multipleOf", [](auto&& self) -> auto& { return self.attributes.multiple_of; }, //
      "minProperties", [](auto&& self) -> auto& { return self.attributes.min_properties; }, //
      "maxProperties", [](auto&& self) -> auto& { return self.attributes.max_properties; }, //
      //    "dependentRequired", [](auto&& self) -> auto& { return self.attributes.dependent_required; }, //
      //      "required", [](auto&& self) -> auto& { return self.attributes.required; }, //
      "minItems", [](auto&& self) -> auto& { return self.attributes.min_items; }, //
      "maxItems", [](auto&& self) -> auto& { return self.attributes.max_items; }, //
      "minContains", [](auto&& self) -> auto& { return self.attributes.min_contains; }, //
      "maxContains", [](auto&& self) -> auto& { return self.attributes.max_contains; }, //
      "uniqueItems", [](auto&& self) -> auto& { return self.attributes.unique_items; }, //
      "x-tfc", [](auto&& self) -> auto& { return self.attributes.tfc_metadata; }
  );
  // clang-format on
};

template <>
struct glz::meta<tfc::json::defined_formats> {
  using enum tfc::json::defined_formats;
  static constexpr std::string_view name = "defined_formats";
  // clang-format off
  static constexpr auto value = glz::enumerate(
      "date-time", datetime,
      "date", date,
      "time", time,
      "duration", duration,
      "email", email,
      "idn-email", idn_email,
      "hostname", hostname,
      "idn-hostname", idn_hostname,
      "ipv4", ipv4,
      "ipv6",ipv6,
      "uri", uri,
      "uri-reference", uri_reference,
      "iri",iri,
      "iri-reference", iri_reference,
      "uuid", uuid,
      "uri-template", uri_template,
      "json-pointer", json_pointer,
      "relative-json-pointer", relative_json_pointer,
      "regex", regex);
  // clang-format on
};

namespace tfc::json {
namespace detail {
template <class T = void>
struct to_json_schema {
  template <auto Opts>
  static void op(auto& s, auto& defs) noexcept {
    if constexpr (glz::detail::glaze_t<T> && std::is_member_object_pointer_v<glz::meta_wrapper_t<T>>) {  // &T::member
      using val_t = glz::detail::member_t<T, glz::meta_wrapper_t<T>>;
      to_json_schema<val_t>::template op<Opts>(s, defs);
    } else if constexpr (glz::detail::glaze_const_value_t<T>) {  // &T::constexpr_member
      using constexpr_val_t = glz::detail::member_t<T, glz::meta_wrapper_t<T>>;
      static constexpr auto val_v{ *glz::meta_wrapper_v<T> };
      if constexpr (glz::detail::glaze_enum_t<constexpr_val_t>) {
        s.attributes.constant = glz::enum_name_v<val_v>;
      } else {
        // General case, needs to be convertible to schema_any
        s.attributes.constant = val_v;
      }
      to_json_schema<constexpr_val_t>::template op<Opts>(s, defs);
    } else {
      // todo static_assert, it is more beneficial to get compile error instead of default everything
      []<bool flag = false> {
        static_assert(flag, "Please provide a schema type for your given type");
      }
      ();
    }
  }
};

template <class T>
  requires(std::same_as<T, bool> || std::same_as<T, std::vector<bool>::reference> ||
           std::same_as<T, std::vector<bool>::const_reference>)
struct to_json_schema<T> {
  template <auto Opts>
  static void op(auto& s, auto&) noexcept {
    s.type = { "boolean" };
  }
};

template <glz::detail::num_t T>
struct to_json_schema<T> {
  template <auto Opts>
  static void op(auto& s, auto&) noexcept {
    if constexpr (std::integral<T>) {
      s.type = { "integer" };
      s.attributes.minimum = static_cast<std::int64_t>(std::numeric_limits<T>::lowest());
      s.attributes.maximum = static_cast<std::uint64_t>(std::numeric_limits<T>::max());
    } else {
      s.type = { "number" };
      s.attributes.minimum = std::numeric_limits<T>::lowest();
      s.attributes.maximum = std::numeric_limits<T>::max();
    }
  }
};

template <class T>
  requires glz::detail::str_t<T> || glz::detail::char_t<T>
struct to_json_schema<T> {
  template <auto Opts>
  static void op(auto& s, auto&) noexcept {
    s.type = { "string" };
  }
};

template <glz::detail::always_null_t T>
struct to_json_schema<T> {
  template <auto Opts>
  static void op(auto& s, auto&) noexcept {
    s.type = { "null" };
    s.attributes.constant = std::monostate{};
  }
};

template <glz::detail::glaze_enum_t T>
struct to_json_schema<T> {
  template <auto Opts>
  static void op(auto& s, auto&) noexcept {
    s.type = { "string" };

    // TODO use oneOf instead of enum to handle doc comments
    using V = std::decay_t<T>;
    static constexpr auto N = std::tuple_size_v<glz::meta_t<V>>;
    // s.enumeration = std::vector<std::string_view>(N);
    // for_each<N>([&](auto I) {
    //    static constexpr auto item = std::get<I>(meta_v<V>);
    //    (*s.enumeration)[I.value] = std::get<0>(item);
    // });
    s.oneOf = std::vector<schematic>(N);
    glz::for_each<N>([&](auto I) {
      // static constexpr auto item = glz::tuplet::get<I>(glz::meta_v<V>);
      static constexpr auto item = glz::meta_v<V>[I];
      auto& enumeration = (*s.oneOf)[I.value];
      // enumeration.attributes.constant = glz::tuplet::get<0>(item);
      enumeration.attributes.constant = item[0];
      // enumeration.attributes.title = glz::tuplet::get<0>(item);
      enumeration.attributes.title = item[0];
      if constexpr (std::tuple_size_v<decltype(item)> > 2) {
        // using additional_data_type = decltype(glz::tuplet::get<2>(item));
        using additional_data_type = decltype(item[2]);
        if constexpr (std::is_convertible_v<additional_data_type, std::string_view>) {
          // enumeration.attributes.description = glz::tuplet::get<2>(item);
          enumeration.attributes.description = item[2];
        } else if constexpr (std::is_convertible_v<additional_data_type, schema>) {
          // enumeration.attributes = glz::tuplet::get<2>(item);
          enumeration.attributes = item[2];
        }
      }
    });
  }
};

template <class T>
struct to_json_schema<glz::basic_raw_json<T>> {
  template <auto Opts>
  static void op(auto& s, auto&) noexcept {
    s.type = { "number", "string", "boolean", "object", "array", "null" };
  }
};

template <glz::detail::array_t T>
struct to_json_schema<T> {
  template <auto Opts>
  static void op(auto& s, auto& defs) noexcept {
    using V = std::decay_t<glz::range_value_t<std::decay_t<T>>>;
    s.type = { "array" };
    auto& def = defs[glz::name_v<V>];
    if (!def.type) {
      to_json_schema<V>::template op<Opts>(def, defs);
    }
    s.items = schema{ glz::detail::join_v<glz::chars<"#/$defs/">, glz::name_v<V>> };
  }
};

template <glz::detail::writable_map_t T>
struct to_json_schema<T> {
  template <auto Opts>
  static void op(auto& s, auto& defs) noexcept {
    using V = std::decay_t<std::tuple_element_t<1, glz::range_value_t<std::decay_t<T>>>>;
    s.type = { "object" };
    auto& def = defs[glz::name_v<V>];
    if (!def.type) {
      to_json_schema<V>::template op<Opts>(def, defs);
    }
    s.additionalProperties = schema{ glz::detail::join_v<glz::chars<"#/$defs/">, glz::name_v<V>> };
  }
};

template <glz::detail::nullable_t T>
struct to_json_schema<T> {
  template <auto Opts>
  static void op(auto& s, auto& defs) noexcept {
    using V = std::decay_t<decltype(*std::declval<std::decay_t<T>>())>;
    to_json_schema<V>::template op<Opts>(s, defs);
    (*s.type).emplace_back("null");
  }
};

template <glz::is_variant T>
struct to_json_schema<T> {
  template <auto Opts>
  static void op(auto& s, auto& defs) noexcept {
    static constexpr auto N = std::variant_size_v<T>;
    s.type = { "number", "string", "boolean", "object", "array", "null" };
    s.oneOf = std::vector<schematic>(N);
    glz::for_each<N>([&](auto I) {
      using V = std::decay_t<std::variant_alternative_t<I, T>>;
      auto& schema_val = (*s.oneOf)[I.value];
      schema_val.attributes.title = glz::name_v<V, true>;  // please MAKE sure you declare name within variants
      // TODO use ref to avoid duplication in schema
      to_json_schema<V>::template op<Opts>(schema_val, defs);
      constexpr bool glaze_object = glz::detail::glaze_object_t<V>;
      if constexpr (glaze_object) {
        auto& def = defs[glz::name_v<std::string>];
        if (!def.type) {
          to_json_schema<std::string>::template op<Opts>(def, defs);
        }
        if constexpr (!glz::tag_v<T>.empty()) {
          (*schema_val.properties)[glz::tag_v<T>] =
              schema{ glz::detail::join_v<glz::chars<"#/$defs/">, glz::name_v<std::string>> };
          // TODO use enum or oneOf to get the ids_v to validate type name
        }
      }
    });
  }
};

template <class T>
  requires glz::detail::glaze_array_t<std::decay_t<T>> || glz::detail::tuple_t<std::decay_t<T>>
struct to_json_schema<T> {
  template <auto Opts>
  static void op(auto& s, auto&) noexcept {
    // TODO: Actually handle this. We can specify a schema per item in items
    //      We can also do size restrictions on static arrays
    s.type = { "array" };
  }
};

template <glz::detail::glaze_object_t T>
struct to_json_schema<T> {
  template <auto Opts>
  static void op(auto& s, auto& defs) noexcept {
    s.type = { "object" };

    using V = std::decay_t<T>;
    static constexpr auto N = std::tuple_size_v<glz::meta_t<V>>;
    s.properties = std::map<std::string_view, schema, std::less<>>();
    glz::for_each<N>([&](auto I) {
      static constexpr auto item = glz::meta_v<V>[I];
      using val_t = std::decay_t<glz::detail::member_t<V, decltype(item)>>;
      auto& def = defs[glz::name_v<val_t>];
      auto ref_val = schema{ glz::detail::join_v<glz::chars<"#/$defs/">, glz::name_v<val_t>> };
      // clang-format off
      if constexpr (std::tuple_size_v<decltype(item)> > 2) {
        // clang-format on
        using additional_data_type = decltype(item[2]);
        if constexpr (std::is_convertible_v<additional_data_type, std::string_view>) {
          ref_val.description = item[2];
        } else if constexpr (std::is_convertible_v<additional_data_type, schema>) {
          ref_val = item[2];
          ref_val.ref = glz::detail::join_v<glz::chars<"#/$defs/">, glz::name_v<val_t>>;
        }
      }
      (*s.properties)[item] = ref_val;

      if (!def.type) {
        to_json_schema<val_t>::template op<Opts>(def, defs);
      }
    });
    s.additionalProperties = false;
  }
};

}  // namespace detail

template <class T, class Buffer>
inline void write_json_schema(Buffer&& buffer) noexcept {
  detail::schematic s{};
  s.defs.emplace();
  detail::to_json_schema<std::decay_t<T>>::template op<glz::opts{}>(s, *s.defs);
  write<glz::opts{}>(std::move(s), std::forward<Buffer>(buffer));
}

template <class T>
inline auto write_json_schema() noexcept {
  std::string buffer{};
  detail::schematic s{};
  s.defs.emplace();
  detail::to_json_schema<std::decay_t<T>>::template op<glz::opts{}>(s, *s.defs);
  glz::write<glz::opts{ .write_type_info = false }>(std::move(s), buffer);
  return buffer;
}
}  // namespace tfc::json
