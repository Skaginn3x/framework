#!/bin/bash
dbus-daemon --system

sed -i 's|deny own=|allow own=|g' /usr/share/dbus-1/system.conf
sed -i 's|deny send_type="method_call"|allow send_type="method_call"|g' /usr/share/dbus-1/system.conf
dbus-send --system --type=method_call --dest=org.freedesktop.DBus / org.freedesktop.DBus.ReloadConfig
