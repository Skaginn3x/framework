#pragma once

#include <algorithm>
#include <regex>
#include <string>
#include <string_view>

#include <fmt/format.h>

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
/// \throws exception::invalid_name if input contains `-` or `//`
auto make_dbus_name(std::string_view input_name) -> std::string;

/// \brief make dbus path like /org/freedesktop/<service_name>
/// \param input_name name postfix
/// \note the prefix is cmake configure option, refer to libs/configure_options for more info
/// \throws exception::invalid_name if input contains `-` or `//`
/// Will replace all `.` with `/`
auto make_dbus_path(std::string_view input_name) -> std::string;

template <typename... args_t>
auto make_dbus_path(fmt::format_string<args_t...> fmt_literal, args_t&&... args) -> std::string {
  return make_dbus_path(fmt::format(fmt_literal, std::forward<args_t>(args)...));
}

/// \brief make dbus name like org.freedesktop.<service_name>
/// utilizes tfc::base to relize required information
auto make_dbus_process_name() -> std::string;

/// \brief strip dbus name like org.freedesktop.<service_name> to <service_name>
/// \return remove detail::dbus_name_prefix from input parameter
/// \note if input parameter does not start with detail::dbus_name_prefix, returns input parameter
auto strip_dbus_name(std::string_view input_name) -> std::string;

}  // namespace tfc::dbus
