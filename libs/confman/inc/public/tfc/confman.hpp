#pragma once
#include <functional>
#include <type_traits>
#include <string_view>

constexpr std::string_view confman_socket = "/var/run/confman.sock";

namespace tfc::confman {
template<typename conf_param_t>
concept non_floating = std::equality_comparable<conf_param_t> && !std::is_floating_point_v<conf_param_t>;

template<non_floating conf_param_t>
class on_change {
public:
  on_change(const conf_param_t&& value, std::function< void (conf_param_t)>&& callback) : value_{std::move(value)}, callback_{std::move(callback)} {}
  void set(const conf_param_t&& value){
    if (value != value_){
      value_ = std::move(value);
      callback_(value_);
    }
  }
  auto value() const noexcept -> conf_param_t const & { return value_; }
private:
  conf_param_t value_;
  std::function<void(conf_param_t const&)> callback_;
};

template<typename service_t>
concept service = std::is_class_v<service_t> && std::is_trivially_copyable_v<service_t> && std::is_default_constructible_v<service_t>;

template<service service_t>
class confman {
public:
  confman() = default;

  void register_service(service_t to_register) {}
};
}  // namespace tfc::confman
