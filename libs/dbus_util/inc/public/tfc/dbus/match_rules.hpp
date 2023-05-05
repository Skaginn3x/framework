#pragma once

#include <string_view>
#include <tfc/stx/string_view_join.hpp>

namespace tfc::dbus::match::rules {
namespace type {
/// \brief make dbus `type` match rule for signal
/// \note Match on the message type.
static constexpr std::string_view signal{ "type='signal'," };
/// \brief make dbus `type` match rule for method
/// \note Match on the message type.
static constexpr std::string_view method{ "type='method'," };
/// \brief make dbus `type` match rule for method_return
/// \note Match on the message type.
static constexpr std::string_view method_return{ "type='method_return'," };
/// \brief make dbus `type` match rule for error
/// \note Match on the message type.
static constexpr std::string_view error{ "type='error'," };
}  // namespace type
namespace detail {
static constexpr std::string_view quote{ "'" };
static constexpr std::string_view comma{ "," };
static constexpr std::string_view sender_prefix{ "sender=" };
static constexpr std::string_view interface_prefix{ "interface=" };
static constexpr std::string_view member_prefix{ "member=" };
static constexpr std::string_view path_prefix{ "path=" };
static constexpr std::string_view path_namespace_prefix{ "path_namespace=" };
static constexpr std::string_view destination_prefix{ "destination=" };

template <std::string_view const& key, std::string_view const& value>
static constexpr std::string_view filter{ stx::string_view_join_v<key, detail::quote, value, detail::quote, detail::comma> };

}  // namespace detail

/// \brief make dbus `sender` match rule
/// \tparam sender_in filter value
/// \note Match messages sent by a particular sender. An example of a sender match is sender='org.freedesktop.Hal'
/// Reference: https://dbus.freedesktop.org/doc/dbus-specification.html
template <std::string_view const& sender_in>
static constexpr std::string_view sender{ detail::filter<detail::sender_prefix, sender_in> };

/// \brief make dbus `interface` match rule
/// \tparam interface_in filter value
/// \note Match messages sent over or to a particular interface.
/// An example of an interface match is interface='org.freedesktop.Hal.Manager'.
/// If a message omits the interface header, it must not match any rule that specifies this key.
/// Reference: https://dbus.freedesktop.org/doc/dbus-specification.html
template <std::string_view const& interface_in>
static constexpr std::string_view interface {
  detail::filter<detail::interface_prefix, interface_in>
};

/// \brief make dbus `member` match rule
/// \tparam member_in filter value
/// \note Matches messages which have the give method or signal name.
/// An example of a member match is member='NameOwnerChanged'
/// Reference: https://dbus.freedesktop.org/doc/dbus-specification.html
template <std::string_view const& member_in>
static constexpr std::string_view member {
  detail::filter<detail::member_prefix, member_in>
};

/// \brief make dbus `path` match rule
/// \tparam path_in filter value
/// \note Match messages sent over or to a particular interface.
/// An example of an interface match is interface='org.freedesktop.Hal.Manager'.
/// If a message omits the interface header, it must not match any rule that specifies this key.
/// Reference: https://dbus.freedesktop.org/doc/dbus-specification.html
template <std::string_view const& path_in>
static constexpr std::string_view path{ detail::filter<detail::path_prefix, path_in> };

/// \brief make dbus `path_namespace` match rule
/// \tparam path_namespace_in filter value
/// \note Matches messages which are sent from or to an object for which the object path is either the given value,
/// or that value followed by one or more path components.
/// For example, path_namespace='/com/example/foo' would match signals sent by /com/example/foo or by /com/example/foo/bar,
/// but not by /com/example/foobar.
/// Using both path and path_namespace in the same match rule is not allowed.
/// Reference: https://dbus.freedesktop.org/doc/dbus-specification.html
template <std::string_view const& path_namespace_in>
static constexpr std::string_view path_namespace{ detail::filter<detail::path_namespace_prefix, path_namespace_in> };

/// \brief make dbus `destination` match rule
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
static_assert("member='foo'," == member<foo>);
static_assert("path='foo'," == path<foo>);
static_assert("path_namespace='foo'," == path_namespace<foo>);
static_assert("destination='foo'," == destination<foo>);

}  // namespace test

}  // namespace tfc::dbus::match::rules
