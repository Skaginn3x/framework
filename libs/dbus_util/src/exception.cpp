#include <tfc/dbus/exception.hpp>

namespace tfc::dbus::exception {

// store invalid_name vtable only in this translation unit, warning weak-vtable
invalid_name::~invalid_name() {}

}  // namespace tfc::dbus::exception
