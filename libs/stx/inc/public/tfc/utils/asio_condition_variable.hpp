#pragma once

#include <cstddef>
#include <memory>
#include <system_error>

#include <boost/asio/async_result.hpp>
#include <boost/asio/steady_timer.hpp>

namespace tfc::asio {

namespace asio = boost::asio;

template <typename executor_t>
class condition_variable {
public:
  using executor_type = executor_t;
  using timer_type = asio::system_timer;
  using time_point_type = typename timer_type::time_point;
  using duration_type = typename timer_type::duration;

  explicit condition_variable(executor_t&& executor)
      : timer_{ std::make_shared<timer_type>(std::move(executor), time_point_type::max()) } {}
  explicit condition_variable(executor_t const& executor)
      : timer_{ std::make_shared<timer_type>(executor, time_point_type::max()) } {}

  condition_variable(condition_variable const&) = delete;
  auto operator=(condition_variable const&) -> condition_variable& = delete;
  condition_variable(condition_variable&& other) noexcept
      : handlers_to_be_notified_{ other.handlers_to_be_notified_ }, timer_{ other.timer_ } {}
  auto operator=(condition_variable&& other) noexcept -> condition_variable& {
    handlers_to_be_notified_ = other.handlers_to_be_notified_;
    timer_ = other.timer_;
    return *this;
  }
  ~condition_variable() = default;

  auto get_executor() -> decltype(auto) { return timer_->get_executor(); }

  /// \param token completion token to be triggered when notify is called
  auto async_wait(asio::completion_token_for<void(std::error_code)> auto&& token) {
    // capturing `this` is UB when moving the condition_variable
    return asio::async_compose<decltype(token), void(std::error_code)>(
        [timer = timer_, handlers_to_be_notified = handlers_to_be_notified_, first_call = true](
            auto& self, std::error_code err = {}) mutable {
          if (first_call) {
            first_call = false;
            timer->async_wait(std::move(self));
            return;
          }
          if (*handlers_to_be_notified == 0) {
            // propagate unexpected errors like unintended cancels
            self.complete(err);
            return;  // prevent unintended underflow
          }
          if ((*handlers_to_be_notified)-- > 0) {
            // if notify is active than it means the user has asked for a notification
            // for reference notifications are propagated as a cancellation of the endless timer
            self.complete(std::error_code{});
            return;
          }
          self.complete(err);
        },
        token, get_executor());
  }

  /// \param err boost system error_code to be propagated to the handlers
  /// \return number of triggered handlers either 0 or 1
  auto notify_one(auto& err) -> std::size_t {
    auto const res{ timer_->cancel_one(err) };
    (*handlers_to_be_notified_) += res;
    return res;
  }

  /// \throws boost::system::system_error Thrown on failure
  /// \return number of triggered handlers either 0 or 1
  auto notify_one() -> std::size_t {
    std::size_t const res{ timer_->cancel_one() };
    (*handlers_to_be_notified_) += res;
    return res;
  }

  /// \param err boost system error_code to be propagated to the handlers
  /// \return number of triggered handlers
  auto notify_all(auto& err) -> std::size_t {
    auto const res{ timer_->cancel(err) };
    (*handlers_to_be_notified_) += res;
    return res;
  }

  /// \throws boost::system::system_error Thrown on failure
  /// \return number of triggered handlers
  auto notify_all() -> std::size_t {
    auto const res{ timer_->cancel() };
    (*handlers_to_be_notified_) += res;
    return res;
  }

private:
  // shared_ptr to simplify move semantics
  std::shared_ptr<std::size_t> handlers_to_be_notified_{ std::make_shared<std::size_t>(0) };
  std::shared_ptr<timer_type> timer_;
};

template <typename executor_t>
condition_variable(executor_t&&) -> condition_variable<executor_t>;

}  // namespace tfc::asio
