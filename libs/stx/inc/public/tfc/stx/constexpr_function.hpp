#pragma once

#include <memory>

#include <tfc/utils/pragmas.hpp>

// from https://github.com/tip-of-the-week/cpp/blob/master/tips/318.md

namespace tfc::stx {

template <class>
struct function;

template <class R, class... Args>
struct function<R(Args...)> {
  template <class F>
  constexpr function(F f) : ptr_{ std::make_unique<implementation<F>>(f) } {}

  constexpr auto operator()(Args... args) const -> R {
    // todo can we get rid of this warning suppression?
    // clang-format off
    PRAGMA_CLANG_WARNING_PUSH_OFF(-Wundefined-func-template)
    return ptr_->get(args...);
    PRAGMA_CLANG_WARNING_POP
    // clang-format on
  }

private:
  struct interface {
    constexpr virtual auto get(Args...) -> R = 0;
    constexpr virtual ~interface() = default;
  };

  template <class F>
  struct implementation final : interface {
    constexpr explicit(true) implementation(F f) : f_{ f } {}
    constexpr auto get(Args... args) -> R override { return f_(args...); }

  private:
    F f_;
  };

  std::unique_ptr<interface> ptr_;
};

// https://en.cppreference.com/w/cpp/utility/functional/function/deduction_guides

template <class>
struct function_traits {};

template <class R, class G, class... A>
struct function_traits<R (G::*)(A...) const> {
  using function_type = R(A...);
};

template <class F>
using function_type_t = typename function_traits<F>::function_type;

// This overload participates in overload resolution only if &F::operator() is
// well-formed when treated as an unevaluated operand and
// decltype(&F::operator()) is of the form R(G::*)(A...) (optionally
// cv-qualified, optionally noexcept, optionally lvalue reference qualified).
// The deduced type is std::function<R(A...)>.
template <class F>
function(F) -> function<function_type_t<decltype(&F::operator())>>;

namespace test {

consteval auto test_empty() {
  function f = [] { return 42; };
  return f();
}

consteval auto test_arg() {
  function f = [](int i) { return i; };
  return f(42);
}

consteval auto test_capture() {
  int i = 42;
  function f = [&] { return i; };
  return f();
}

static_assert(42 == test_empty());
static_assert(42 == test_arg());
static_assert(42 == test_capture());

}  // namespace test

}  // namespace tfc::stx
