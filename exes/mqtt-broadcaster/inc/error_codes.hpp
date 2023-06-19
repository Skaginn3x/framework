#pragma once

#include <async_mqtt/all.hpp>

namespace detail {

class suback_error_category : public std::error_category {
public:
  virtual const char* name() const noexcept override { return "async_mqtt::suback_error_category"; }
  virtual std::string message(int value) const override {
    return suback_reason_code_to_str(static_cast<async_mqtt::suback_reason_code>(value));
  }
};

}  // namespace detail

const std::error_category& suback_error_category() {
  static detail::suback_error_category instance;
  return instance;
}

inline std::error_code make_error_code(async_mqtt::suback_reason_code v) {
  return std::error_code(static_cast<uint8_t>(v), suback_error_category());
}
