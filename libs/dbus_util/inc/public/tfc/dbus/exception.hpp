#pragma once

#include <sdbusplus/exception.hpp>

#define EXPORT __attribute__((visibility("default")))

namespace tfc::dbus::exception {
class runtime : public sdbusplus::exception::internal_exception {
public:
  runtime(const std::string& description);

  const char* name() const noexcept final;
  const char* description() const noexcept final;
  const char* what() const noexcept final;
  int get_errno() const noexcept final;

private:
  std::string description_;
};
}  // namespace tfc::dbus::exception
