#pragma once

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

static constexpr std::string_view dbus_path_prefix{
  stx::string_view_join_v<detail::slash, configure_options::dbus_domain, detail::slash, configure_options::dbus_company, detail::slash>
};

template<std::string_view const& prefix>
auto constexpr make(std::string_view service_name) -> std::string {
  return std::string(prefix.data(), prefix.size()) + std::string(service_name.data(), service_name.size());
}

}  // namespace detail

/// \brief make dbus name like org.freedesktop.<input_name>
/// \tparam input_name name postfix
/// \note the prefix is cmake configure option, refer to libs/configure_options for more info
template<std::string_view const& input_name>
static auto constexpr const_dbus_name{ stx::string_view_join_v<detail::dbus_name_prefix, input_name> };

/// \brief make dbus path like /org/freedesktop/<service_name>
/// \tparam input_name name postfix
/// \note the prefix is cmake configure option, refer to libs/configure_options for more info
template<std::string_view const& input_name>
static auto constexpr const_dbus_path{ stx::string_view_join_v<detail::dbus_path_prefix, input_name> };

/// \brief make dbus name like org.freedesktop.<service_name>
/// \param input_name name postfix
/// \note the prefix is cmake configure option, refer to libs/configure_options for more info
auto constexpr make_dbus_name(std::string_view input_name) -> std::string {
  return detail::make<detail::dbus_name_prefix>(input_name);
}

/// \brief make dbus path like /org/freedesktop/<service_name>
/// \param input_name name postfix
/// \note the prefix is cmake configure option, refer to libs/configure_options for more info
auto constexpr make_dbus_path(std::string_view input_name) -> std::string {
  return detail::make<detail::dbus_path_prefix>(input_name);
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

/// \brief make dbus header `sender` key filter
/// \tparam sender_in filter value
/// \note Unique name of the sending connection.
/// This field is usually only meaningful in combination with the message bus,
/// but other servers may define their own meanings for it.
/// On a message bus, this header field is controlled by the message bus,
/// so it is as reliable and trustworthy as the message bus itself. Otherwise,
/// this header field is controlled by the message sender,
/// unless there is out-of-band information that indicates otherwise.
/// Reference: https://dbus.freedesktop.org/doc/dbus-specification.html
template <std::string_view const& sender_in>
static constexpr std::string_view sender{ detail::filter<detail::sender_prefix, sender_in> };

/// \brief make dbus header `interface` key filter
/// \tparam interface_in filter value
/// \note Match messages sent by a particular sender. An example of a sender match is sender='org.freedesktop.Hal'
/// Reference: https://dbus.freedesktop.org/doc/dbus-specification.html
template <std::string_view const& interface_in>
static constexpr std::string_view interface {
  detail::filter<detail::interface_prefix, interface_in>
};

/// \brief make dbus header `path` key filter
/// \tparam path_in filter value
/// \note Match messages sent over or to a particular interface.
/// An example of an interface match is interface='org.freedesktop.Hal.Manager'.
/// If a message omits the interface header, it must not match any rule that specifies this key.
/// Reference: https://dbus.freedesktop.org/doc/dbus-specification.html
template <std::string_view const& path_in>
static constexpr std::string_view path{ detail::filter<detail::path_prefix, path_in> };

/// \brief make dbus header `path_namespace` key filter
/// \tparam path_namespace_in filter value
/// \note Matches messages which are sent from or to an object for which the object path is either the given value,
/// or that value followed by one or more path components.
/// For example, path_namespace='/com/example/foo' would match signals sent by /com/example/foo or by /com/example/foo/bar,
/// but not by /com/example/foobar.
/// Using both path and path_namespace in the same match rule is not allowed.
/// Reference: https://dbus.freedesktop.org/doc/dbus-specification.html
template <std::string_view const& path_namespace_in>
static constexpr std::string_view path_namespace{ detail::filter<detail::path_namespace_prefix, path_namespace_in> };

/// \brief make dbus header `destination` key filter
/// \tparam destination_in filter value
/// \note Matches messages which are being sent to the given unique name.
/// An example of a destination match is destination=':1.0'
/// Reference: https://dbus.freedesktop.org/doc/dbus-specification.html
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
