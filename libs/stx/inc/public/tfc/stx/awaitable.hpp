#pragma once

#include <coroutine>
#include <exception>
#include <optional>
#include <tuple>

namespace tfc::stx {

template <typename T>
struct async_result {
  std::optional<T> value{};
  std::exception_ptr exception{};
  std::coroutine_handle<> coro{};
};
template <typename T, std::invocable Func, typename... Args>
struct async_task {
  Func func{};
  std::tuple<Args...> args{};
  async_result<T>& result{};

  async_task(Func&& f, async_result<T>& r, Args&&... a)
      : func(std::forward<Func>(f)), args(std::forward<Args>(a)...), result(r) {}

  void operator()() {
    try {
      std::apply([&](auto&&... a) { result.value.emplace(func(std::forward<decltype(a)>(a)...)); }, args);
    } catch (...) {
      result.exception = std::current_exception();
    }
    result.coro.resume();
  }
};
template <typename T>
struct async_awaitable {
  async_result<T>& result;

  [[nodiscard]] bool await_ready() const noexcept { return !result.coro || result.value.has_value() || result.exception; }

  void await_suspend(std::coroutine_handle<> coro) noexcept {
    result.coro = coro;
    if (result.value || result.exception) {
      coro.resume();
    }
  }

  T await_resume() {
    if (result.exception) {
      std::rethrow_exception(result.exception);
    }
    return std::move(*result.value);
  }

  //  template <typename U>
  //  friend async_awaitable<U> operator co_await(async_awaitable<U> awaitable) {
  //    return awaitable;
  //  }
};

}  // namespace tfc::stx
