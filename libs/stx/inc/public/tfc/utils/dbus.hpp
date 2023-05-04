#pragma once

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

static constexpr std::string_view dbus_path_prefix{
  stx::string_view_join_v<detail::slash, configure_options::dbus_domain, detail::slash, configure_options::dbus_company, detail::slash>
};

template<std::string_view const& prefix>
auto constexpr make(std::string_view service_name) -> std::string {
  return std::string(prefix.data(), prefix.size()) + std::string(service_name.data(), service_name.size());
}

}  // namespace detail

template<std::string_view const& service_name>
static auto constexpr const_dbus_name{ stx::string_view_join_v<detail::dbus_name_prefix, service_name> };

template<std::string_view const& service_name>
static auto constexpr const_dbus_path{ stx::string_view_join_v<detail::dbus_path_prefix, service_name> };

auto constexpr make_dbus_name(std::string_view service_name) -> std::string {
  return detail::make<detail::dbus_name_prefix>(service_name);
}

auto constexpr make_dbus_path(std::string_view service_name) -> std::string {
  return detail::make<detail::dbus_path_prefix>(service_name);
}

}  // namespace tfc::dbus

namespace tfc::dbus::match::rules {
namespace type {
static constexpr std::string_view signal{ "type='signal'," };
static constexpr std::string_view method{ "type='method'," };
static constexpr std::string_view method_return{ "type='method_return'," };
static constexpr std::string_view error{ "type='error'," };
}  // namespace type
namespace detail {
static constexpr std::string_view quote{ "'" };
static constexpr std::string_view comma{ "," };
static constexpr std::string_view sender_prefix{ "sender=" };
static constexpr std::string_view interface_prefix{ "interface=" };
static constexpr std::string_view path_prefix{ "path=" };
static constexpr std::string_view path_namespace_prefix{ "path_namespace=" };
static constexpr std::string_view destination_prefix{ "destination=" };

template <std::string_view const& key, std::string_view const& value>
static constexpr std::string_view filter{ stx::string_view_join_v<key, detail::quote, value, detail::quote, detail::comma> };

}  // namespace detail

template <std::string_view const& sender_in>
static constexpr std::string_view sender{ detail::filter<detail::sender_prefix, sender_in> };

template <std::string_view const& interface_in>
static constexpr std::string_view interface {
  detail::filter<detail::interface_prefix, interface_in>
};

template <std::string_view const& path_in>
static constexpr std::string_view path{ detail::filter<detail::path_prefix, path_in> };

template <std::string_view const& path_namespace_in>
static constexpr std::string_view path_namespace{ detail::filter<detail::path_namespace_prefix, path_namespace_in> };

template <std::string_view const& destination_in>
static constexpr std::string_view destination{ detail::filter<detail::destination_prefix, destination_in> };

namespace test {

static constexpr std::string_view foo{ "foo" };
static_assert("sender='foo'," == sender<foo>);
static_assert("interface='foo'," == interface<foo>);
static_assert("path='foo'," == path<foo>);
static_assert("path_namespace='foo'," == path_namespace<foo>);
static_assert("destination='foo'," == destination<foo>);

}  // namespace test

}  // namespace tfc::dbus::match::rules
