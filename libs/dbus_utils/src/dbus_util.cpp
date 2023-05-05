#include "tfc/dbus_util.hpp"

namespace tfc::dbus::exception {
runtime::runtime(const std::string& description) : description_{ description } {}

const char* runtime::name() const noexcept {
  return "com.skaginn3x.Error.runtimeError";
}
const char* runtime::description() const noexcept {
  return description_.data();
}

// Implemented here to complete the base class
// Unused by the dbus server for setting errors
int runtime::get_errno() const noexcept {
  return 0;
}
const char* runtime::what() const noexcept {
  return description_.data();
}
}  // namespace tfc::dbus::exception
