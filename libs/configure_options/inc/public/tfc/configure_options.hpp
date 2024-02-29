#pragma once

#include <string_view>

namespace tfc::configure_options {

static constexpr std::string_view dbus_domain{ "com" };         // "org" or "com" etc.
static constexpr std::string_view dbus_company{ "skaginn3x" };  // name of company

static_assert(!dbus_domain.empty());
static_assert(!dbus_company.empty());

}  // namespace tfc::configure_options
