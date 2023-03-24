#pragma once

#include <concepts>
#include <functional>
#include <type_traits>

namespace tfc::confman {
template <typename conf_param_t>
concept observable_type = std::equality_comparable<conf_param_t> && !
std::is_floating_point_v<conf_param_t>&& std::is_default_constructible_v<conf_param_t>;

/// \brief observable variable, the user can get notified if it is changed
/// \tparam conf_param_t equality comparable and default constructible type
template <observable_type conf_param_t>
class [[nodiscard]] observable {
public:
  observable() = default;
  explicit observable(conf_param_t&& value) : value_{ std::forward<decltype(value)>(value) } {}
  observable(conf_param_t&& value, std::function<void(conf_param_t)>&& callback)
      : value_{ std::forward<decltype(value)>(value) }, callback_{ std::move(callback) } {}

  observable(observable const&) = default;
  auto operator=(observable const&) -> observable& = default;
  observable(observable&&) noexcept = default;
  auto operator=(observable&&) noexcept -> observable& = default;

  void observe(std::invocable<conf_param_t const&> auto&& callback) const {
    callback_ = std::forward<decltype(callback)>(callback);
  }

  /// \brief set new value, if changed notify observer
  void set(conf_param_t&& value) {
    if (value != value_) {
      value_ = std::forward<decltype(value)>(value);
      callback_(value_);
    }
  }

  /// \brief set new value, if changed notify observer
  auto operator=(conf_param_t&& value) -> observable& {
    set(std::forward<decltype(value)>(value));
    return *this;
  }

  /// \brief get the current value
  auto value() const noexcept -> conf_param_t const& { return value_; }

private:
  conf_param_t value_{};
  mutable std::function<void(conf_param_t const&)> callback_{ [](conf_param_t const&) {} };

public:
  struct glaze {
    static auto constexpr value = &observable::value_;
  };
};

}  // namespace tfc::confman
