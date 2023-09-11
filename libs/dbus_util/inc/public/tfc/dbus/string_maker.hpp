#pragma once

#include <algorithm>
#include <regex>
#include <string>
#include <string_view>

#include <tfc/configure_options.hpp>
#include <tfc/stx/string_view_join.hpp>

namespace tfc::dbus {

namespace detail {
static constexpr std::string_view dot{ "." };
static constexpr std::string_view slash{ "/" };

static constexpr std::string_view dbus_name_prefix{
  stx::string_view_join_v<configure_options::dbus_domain, detail::dot, configure_options::dbus_company, detail::dot>
};

static constexpr std::string_view dbus_path_prefix{ stx::string_view_join_v<detail::slash,
                                                                            configure_options::dbus_domain,
                                                                            detail::slash,
                                                                            configure_options::dbus_company,
                                                                            detail::slash> };

template <std::string_view const& prefix>
auto constexpr make(std::string_view service_name) -> std::string {
  return std::string(prefix.data(), prefix.size()) + std::string(service_name.data(), service_name.size());
}

}  // namespace detail

/// \brief make dbus name like org.freedesktop.<input_name>
/// \tparam input_name name postfix
/// \note the prefix is cmake configure option, refer to libs/configure_options for more info
template <std::string_view const& input_name>
static auto constexpr const_dbus_name{ stx::string_view_join_v<detail::dbus_name_prefix, input_name> };

/// \brief make dbus path like /org/freedesktop/<service_name>
/// \tparam input_name name postfix
/// \note the prefix is cmake configure option, refer to libs/configure_options for more info
template <std::string_view const& input_name>
static auto constexpr const_dbus_path{ stx::string_view_join_v<detail::dbus_path_prefix, input_name> };

/// \brief make dbus name like org.freedesktop.<service_name>
/// \param input_name name postfix
/// \note the prefix is cmake configure option, refer to libs/configure_options for more info
/// Will replace illegal characters with legal ones, '/' to '.', '-' to '_', and .<number> to _<number>
auto constexpr make_dbus_name(std::string_view input_name) -> std::string {
  auto temporary{ detail::make<detail::dbus_name_prefix>(input_name) };
  std::ranges::replace(temporary, '/', '.');
  std::ranges::replace(temporary, '-', '_');
  static constexpr std::string_view change_dots_preceding_number_to{ "(\\.)+(?=\\d)" };
  return std::regex_replace(temporary, std::regex{ change_dots_preceding_number_to.data() }, "_");
}

/// \brief make dbus path like /org/freedesktop/<service_name>
/// \param input_name name postfix
/// \note the prefix is cmake configure option, refer to libs/configure_options for more info
/// Will replace illegal characters with legal ones, '-' to '_' and '.' to '/'
/// Will NOT replace two slashes // with /. Two slashes is illegal
auto constexpr make_dbus_path(std::string_view input_name) -> std::string {
  auto return_value{ detail::make<detail::dbus_path_prefix>(input_name) };
  std::ranges::replace(return_value, '-', '_');
  std::ranges::replace(return_value, '.', '/');
  return return_value;
}

}  // namespace tfc::dbus
