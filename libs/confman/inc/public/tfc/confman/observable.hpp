#pragma once

#include <compare>
#include <concepts>
#include <functional>
#include <type_traits>

#include <fmt/printf.h>

#include <tfc/stx/concepts.hpp>

namespace tfc::confman {
template <typename conf_param_t>
concept observable_type = requires {
                            requires std::is_default_constructible_v<conf_param_t>;
                            requires std::equality_comparable<conf_param_t>;
                            //  requires std::three_way_comparable<conf_param_t>;
                            requires !std::is_floating_point_v<conf_param_t>;  // todo why not
                          };

/// \brief observable variable, the user can get notified if it is changed
/// \tparam conf_param_t equality comparable and default constructible type
/// Limitations:
///   1. recurrent observable ownership like
///     struct foo{ observable<int> integer{}; };
///     struct bar{ observable<foo> instance{}; };
///     // given that you have already set observing callbacks you would only get callback for integer not instance
///     bar{}.instance.integer.set(32);
///     // workaround is to use assignment operators at top level of the recurrence
///     bar obj{}; obj = bar{ .instance = { .integer = 32 } };
template <observable_type conf_param_t>
class [[nodiscard]] observable {
public:
  using callback_t = std::function<void(conf_param_t const& new_value, conf_param_t const& former_value)>;

  observable() = default;

  /// \brief construct observable with default value
  /// \param value default value
  explicit observable(conf_param_t&& value) : value_{ std::move(value) } {}

  /// \brief construct observable with default value
  /// \param value default value
  explicit observable(conf_param_t& value) : value_{ value } {}

  /// \brief construct observable with default value and changes callback
  /// \param value default value
  /// \param callback observer function of type void(conf_param_t const& new_value, conf_param_t const& former_value)
  observable(conf_param_t&& value, tfc::stx::nothrow_invocable<conf_param_t const&, conf_param_t const&> auto&& callback)
      : value_{ std::forward<decltype(value)>(value) }, callback_{ std::forward<decltype(callback)>(callback) } {}

  // todo default copy&move constructor differs from copy&move assignments
  observable(observable const&) = default;
  observable(observable&&) noexcept = default;
  /// \brief copy assignment
  /// Independent side effects:
  /// 1. If copy does not include a valid callback the previous callback is retained.
  /// 2. If value in copy is different from the value in `this` the callback will be called.
  auto operator=(observable const& copy) -> observable& {
    // only set callback if an actual callback is there
    // otherwise we retain the previous callback
    if (copy.callback_) {
      callback_ = copy.callback_;
    }
    auto copy_value = copy.value_;
    set(std::move(copy_value));
    return *this;
  }
  /// \brief move assignment
  /// Independent side effects:
  /// 1. If moved instance does not include a valid callback the previous callback is retained.
  /// 2. If value in moved instance is different from the value in `this` the callback will be called.
  /// 3. If callback will throw it will be silently printed to stderr and continues
  auto operator=(observable&& moveit) noexcept -> observable& {
    // only set callback if an actual callback is there
    // otherwise we retain the previous callback
    if (moveit.callback_) {
      callback_ = std::move(moveit.callback_);
    }
    if (value_ != moveit.value_) {
      try {
        set(std::move(moveit.value_));
      } catch (std::exception const& exc) {
        fmt::fprintf(stderr, "Exception in move constructor of an observable, what: %s. Will fail silently", exc.what());
      }
    }
    return *this;
  }
  /// \brief set new value, if changed notify observer
  auto operator=(conf_param_t&& value) -> observable& {
    set(std::move(value));
    return *this;
  }
  /// \brief set new value, if changed notify observer
  auto operator=(conf_param_t const& value) -> observable& {
    set({ value });
    return *this;
  }

  friend auto constexpr operator==(observable const& lhs, observable const& rhs) noexcept -> bool {
    return lhs.value_ == rhs.value_;
  }
  friend auto constexpr operator==(observable const& lhs, conf_param_t const& rhs) noexcept -> bool {
    return lhs.value_ == rhs;
  }
  // todo how to determine at compile time whether conf_param_t is three_way_comparable
  friend auto constexpr operator<=>(observable const& lhs, observable const& rhs) noexcept {
    return lhs.value_ <=> rhs.value_;
  }
  friend auto constexpr operator<=>(observable const& lhs, conf_param_t const& rhs) noexcept { return lhs.value_ <=> rhs; }

  /// \brief subscribe to changes
  /// \param callback calling function for all changes, this function cannot throw.
  /// \note Reason for callback being nothrow is that when calling multiple observers it is
  ///       not known which observer is called first and it is expected that all observers will be called.
  void observe(tfc::stx::nothrow_invocable<conf_param_t const&, conf_param_t const&> auto&& callback) const {
    callback_ = std::forward<decltype(callback)>(callback);
  }

  /// \brief set new value, if changed notify observer
  template <typename other_conf_paramt_t>
    requires std::same_as<conf_param_t, std::remove_cvref_t<other_conf_paramt_t>>
  void set(other_conf_paramt_t&& new_value) {
    if (new_value != value_) {
      if (callback_) {
        std::invoke(callback_, new_value, value_);
      }
      value_ = std::forward<decltype(new_value)>(new_value);
    }
  }

  /// \brief get the current value
  auto value() const noexcept -> conf_param_t const& { return value_; }

  auto operator->() const noexcept -> decltype(auto) { return std::addressof(value()); }

private:
  conf_param_t value_{};
  mutable std::function<void(conf_param_t const&, conf_param_t const&)> callback_{};

public:
  struct glaze {
    static auto constexpr value = &observable::value_;
    static std::string_view constexpr name{ "observable" };  // todo observable<typeid(conf_param_t)>
  };
};

}  // namespace tfc::confman

namespace glz::detail {

// template <typename value_t>
// struct from_json<tfc::confman::observable<value_t>> {
//   template <auto opts>
//   inline static void op(auto& value, auto&&... args) noexcept {
//     value_t value_copy;
//     read<json>::op<opts>(value_copy, args...);
////    from_json<value_t>::template op<opts>(value_copy, std::forward<decltype(args)>(args)...);
//    value.set(std::move(value_copy)); // invoke callback
//  }
//};

template <typename value_t>
struct to_json_schema<tfc::confman::observable<value_t>> {
  template <auto opts>
  static void op(auto& schema, auto& defs) noexcept {
    to_json_schema<value_t>::template op<opts>(schema, defs);
  }
};

}  // namespace glz::detail
