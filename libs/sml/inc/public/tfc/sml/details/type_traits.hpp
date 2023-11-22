#pragma once

namespace tfc::sml::traits {

template <typename T>
struct sub_sm {};

// need the inner type to be able to get the name
template <typename T>
struct sub_sm<boost::sml::back::sm<boost::sml::back::sm_policy<T>>> {
  using type = T;
};


}  // namespace tfc::sml::traits
