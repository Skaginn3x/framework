#pragma once

import std;
#include <tfc/dbus/string_maker.hpp>

namespace tfc::ipc_ruler::consts {

static constexpr std::string_view dbus_name{ "ipc_ruler" };
static constexpr std::string_view dbus_manager_name{ "manager" };

static constexpr std::string_view signals_property{ "Signals" };
static constexpr std::string_view slots_property{ "Slots" };
static constexpr std::string_view register_signal{ "RegisterSignal" };
static constexpr std::string_view register_slot{ "RegisterSlot" };
static constexpr std::string_view disconnect_method{ "Disconnect" };
static constexpr std::string_view connect_method{ "Connect" };
static constexpr std::string_view connections_property{ "Connections" };
static constexpr std::string_view connection_change{ "ConnectionChange" };

// service name
static constexpr auto ipc_ruler_service_name = dbus::const_dbus_name<dbus_name>;
// object path
static constexpr auto ipc_ruler_object_path = dbus::const_dbus_path<dbus_name>;
// Interface name
static constexpr auto ipc_ruler_interface_name = dbus::const_dbus_name<dbus_manager_name>;

}  // namespace tfc::ipc_ruler::consts
