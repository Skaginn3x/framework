#pragma once

namespace tfc::sml::name {

template <typename type_t>
constexpr auto get_name() -> std::string {
  if constexpr (is_sub_sm<type_t>) {
    using sub_sm = sub_sm<type_t>;
    return get_name<typename sub_sm::type>();
  } else if constexpr (name_exists<type_t>) {
    return std::string{ type_t::name };
  } else {
    return std::string{ boost::sml::aux::string<type_t>{}.c_str() };
  }
}

template <template <typename, typename> typename event_t, typename first_t, typename second_t>
auto constexpr extract_event_type(event_t<first_t, second_t> const&) noexcept -> std::string {
  return get_name<second_t>();
}

template <typename event_t>
auto constexpr extract_event_type(event_t const& event) noexcept -> std::string {
  if constexpr (tfc::stx::is_specialization_v<event_t, boost::sml::back::on_entry> ||
                tfc::stx::is_specialization_v<event_t, boost::sml::back::on_exit>) {
    return detail::extract_event_type(event);
  } else {
    return get_name<event_t>();
  }
}



}  // namespace tfc::sml::name
