#pragma once
#include <functional>
#include <type_traits>
#include <string_view>

constexpr std::string_view confman_socket = "/var/run/confman.sock";

namespace tfc::confman {
template<typename T>
concept non_floating = std::equality_comparable<T> && !std::is_floating_point_v<T>;

template<non_floating T>
class on_change {
public:
  on_change(const T&& value, std::function< void (T)>&& callback) : value_{std::move(value)}, callback_{std::move(callback)} {}
  void set(const T&& value){
    if (value != value_){
      value_ = std::move(value);
      callback_(value_);
    }
  }
  auto value() const noexcept -> T const & { return value_; }
private:
  T value_;
  std::function<void (T)> callback_;
};

template<typename K>
concept service = std::is_class_v<K> && std::is_trivially_copyable_v<K> && std::is_default_constructible_v<K>;

template<service service_t>
class confman {
public:
  confman() = default;

  void register_service(service_t to_register) {}
};
}
