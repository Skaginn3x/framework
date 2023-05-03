#include "tfc/dbus_util.hpp"

tfc::dbus::exception::runtime::runtime(const std::string& description) : description_{ description } {}

const char* tfc::dbus::exception::runtime::name() const noexcept {
  return "com.skaginn3x.Error.runtimeError";
}
const char* tfc::dbus::exception::runtime::description() const noexcept {
  return description_.data();
}

// Implemented here to complete the base class
// Unused by the dbus server for setting errors
int tfc::dbus::exception::runtime::get_errno() const noexcept {
  return 0;
}
const char* tfc::dbus::exception::runtime::what() const noexcept {
  return description_.data();
}
