#pragma once
#include <stdexcept>

#include <fmt/core.h>
#include <sdbusplus/exception.hpp>

namespace tfc::dbus::exception {
class invalid_name final : public std::runtime_error {
public:
  ~invalid_name() override;
  template <typename... args_t>
  invalid_name(fmt::format_string<args_t...> msg, args_t&&... parameters)
      : std::runtime_error{ fmt::vformat(msg, fmt::make_format_args(parameters...)) } {}
};
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
