#pragma once

#include <glaze/glaze.hpp>

namespace tfc::json {

struct schema_attributes {
  std::optional<std::string_view> description{};
  std::optional<std::int64_t> min{};
  std::optional<std::int64_t> max{};
  std::variant<std::monostate, std::string_view, std::int64_t, std::uint64_t, double, bool> default_value{};
};

namespace detail {
using glz::for_each;
using glz::shrink_index_array;
using glz::group_sizes;

template <class Tuple>
constexpr auto filter()
{
  constexpr auto n = std::tuple_size_v<Tuple>;
  std::array<size_t, n> indices{};
  size_t i = 0;
  for_each<n>([&](auto I) {
    using V = std::decay_t<std::tuple_element_t<I, Tuple>>;
//    if constexpr (std::is_member_pointer_v<V>) {
//      []<bool flag = false>(){ static_assert(flag); }();
//    }
    if constexpr (std::convertible_to<V, std::string_view>) {
    }
    else if (std::is_member_pointer_v<V> || std::is_invocable_v<V>) {
      indices[i++] = I - 1;
    }
  });
  return std::make_pair(indices, i);
}

template <class Tuple>
constexpr auto make_groups_helper()
{
  constexpr auto N = std::tuple_size_v<Tuple>;

  constexpr auto filtered = filter<Tuple>();
  constexpr auto starts = shrink_index_array<filtered.second>(filtered.first);
  constexpr auto sizes = group_sizes(starts, N);

  return glz::tuplet::make_tuple(starts, sizes);
}

template <size_t Start, class Tuple, size_t... Is>
constexpr auto make_group(Tuple&& t, std::index_sequence<Is...>)
{
  auto get_elem = [&](auto i) {
    constexpr auto I = decltype(i)::value;
    using type = decltype(glz::tuplet::get<Start + I>(t));
    if constexpr (I == 0 || std::convertible_to<type, std::string_view>) {
      return std::string_view(glz::tuplet::get<Start + I>(t));
    }
    else {
      return glz::tuplet::get<Start + I>(t);
    }
  };
  auto r = glz::tuplet::make_copy_tuple(get_elem(std::integral_constant<size_t, Is>{})...);
  // check_member<decltype(r)>();
  return r;
}

template <auto& GroupStartArr, auto& GroupSizeArr, class Tuple, size_t... GroupNumber>
constexpr auto make_groups_impl(Tuple&& t, std::index_sequence<GroupNumber...>)
{
  return glz::tuplet::make_copy_tuple(make_group<get<GroupNumber>(GroupStartArr)>(
      t, std::make_index_sequence<std::get<GroupNumber>(GroupSizeArr)>{})...);
}

template <typename tuple_t>
struct group_builder
{
  static constexpr auto h = make_groups_helper<tuple_t>();
  static constexpr auto starts = glz::tuplet::get<0>(h);
  static constexpr auto sizes = glz::tuplet::get<1>(h);

  static constexpr auto op(tuple_t&& t)
  {
    constexpr auto n_groups = starts.size();
    return make_groups_impl<starts, sizes>(std::forward<tuple_t>(t), std::make_index_sequence<n_groups>{});
  }
};

}  // namespace detail

static constexpr auto object(auto&&... args) {
  static_assert(sizeof...(args) != 0);
  return glz::detail::Object{detail::group_builder<std::decay_t<decltype(glz::tuplet::make_copy_tuple(args...))>>::op(
      glz::tuplet::make_copy_tuple(args...))};
//  glz::object(args);
}

}  // namespace tfc::json

template<>
struct glz::meta<tfc::json::schema_attributes> {
  using type = tfc::json::schema_attributes;
  static auto constexpr value{ tfc::json::object("description", &type::default_value, "min", &type::min, "max", &type::max, "default", &type::default_value) };
};

