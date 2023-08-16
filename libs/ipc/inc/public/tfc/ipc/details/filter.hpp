#pragma once
#include <boost/asio/io_context.hpp>
#include <concepts>
#include <vector>

//#include <boost/asio.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <glaze/core/common.hpp>

#include <tfc/stx/te.hpp>

namespace tfc::ipc::filter {

namespace asio = boost::asio;

enum struct type_e : std::uint8_t {
  unknown = 0,
  invert,
  timer,
  // todo: https://esphome.io/components/sensor/index.html#sensor-filters
  offset,
  multiply,
  calibrate_linear,
  filter_out,
  median,
  quantile,
  sliding_window_moving_average,
  exponential_moving_average,
  throttle,
  throttle_average,
  delta,
  lambda,
};

template <typename value_t, typename completion_token_t>
struct filter_interface {
  explicit filter_interface(type_e input) : type{ input } {}
  virtual ~filter_interface() = default;
  const type_e type{ type_e::unknown };
  virtual auto async_process(value_t const& value, completion_token_t const& completion_token) const
      -> asio::async_result<std::decay_t<completion_token_t>, void(decltype(value))>::return_type = 0;
protected:
  auto async_process_helper(auto&& process, auto const& completion_token) const {
    auto executor{ asio::get_associated_executor(completion_token) };
    // todo get rid of const_cast
    return asio::async_compose<completion_token_t, void(value_t)>(process, const_cast<completion_token_t&>(completion_token), executor);
  }
};

template <type_e type, typename value_t, typename completion_token_t>
struct filter;

template <typename completion_token_t>
struct filter<type_e::invert, bool, completion_token_t> : filter_interface<bool, completion_token_t> {
  filter() : filter_interface<bool, completion_token_t>{ type_e::invert } {}
  bool invert{ true };
  using value_t = bool;
  using result_t = asio::async_result<std::decay_t<completion_token_t>, void(value_t)>::return_type;
  auto async_process(value_t const& value, completion_token_t const& completion_token) const -> result_t override {
    auto executor{ asio::get_associated_executor(completion_token) };
    auto const impl{ [this, copy = value](auto& self){
      if (invert) {
        self.complete(!copy);
      }
      else {
        self.complete(copy);
      }
    } };
    // todo get rid of const_cast
    return asio::async_compose<completion_token_t, void(bool)>(impl, const_cast<completion_token_t&>(completion_token), executor);
  }

  struct glaze {
    using type = filter<type_e::invert, value_t, completion_token_t>;
    static constexpr auto name{ "ipc::filter::invert" };
    static constexpr auto value{ glz::object("invert", &type::invert, "Invert outputting value") };
  };
};

template <typename value_t, typename completion_token_t>
struct filter<type_e::timer, value_t, completion_token_t> : filter_interface<value_t, completion_token_t> {
  using parent = filter_interface<value_t, completion_token_t>;
  filter() : parent{ type_e::timer } {}
  std::chrono::milliseconds time_on{ 5000 };
  std::chrono::milliseconds time_off{ 0 };

  using result_t = asio::async_result<std::decay_t<completion_token_t>, void(value_t)>::return_type;
  auto async_process(value_t const& value, completion_token_t const& completion_token) const -> result_t override {
    return parent::async_process_helper([this, copy = value, first_call = true](auto& self, std::error_code code = {}) mutable {
      if (code) {
        fmt::print("TODO DO serious business\n ");
        return;
      }
      if (first_call) {
        first_call = false;
        if (timer_) {
          // already waiting need to cancel and begin again
          timer_->cancel();
        }
        auto executor = asio::get_associated_executor(self);
        timer_ = asio::steady_timer{ executor };
        timer_->expires_from_now(time_on);
        timer_->async_wait(std::move(self));
      }
      else {
        self.complete(copy);
      }
    }, completion_token);
  }
private:
  mutable std::optional<asio::steady_timer> timer_{ std::nullopt };

public:
  struct glaze {
    using type = filter<type_e::timer, value_t, completion_token_t>;
    static constexpr auto name{ "ipc::filter::timer" };
    // clang-format off
    static constexpr auto value{ glz::object("time_on", &type::time_on, "Rising edge settling delay",
                                            "time_off", &type::time_off, "Falling edge settling delay") };
    // clang-format on
  };
};

// template <>
// struct filter<type_e::timer> {
//   std::chrono::milliseconds time_on{ 3000 };
//   std::chrono::milliseconds time_off{ 0 };
//
//   auto async_process(auto&& value, auto&& completion_token) const
//       -> asio::async_result<std::decay_t<decltype(completion_token)>, void(decltype(value))>::return_type {
//     auto executor{ asio::get_associated_executor(completion_token) };
//     return asio::async_compose<decltype(completion_token), void(decltype(value))>>
//             ([this, first_call = true, value](auto& self, std::error_code = {}, std::size_t = 0) mutable {
//                                                                    if (first_call) {
//                                                                      first_call = false;
//                                                                      auto executor{ asio::get_associated_executor(self) };
//                                                                      asio::steady_timer timer{ executor };
//                                                                      timer.expires_from_now(time_on);
//                                                                      timer.async_wait(std::move(self));
//                                                                    }
//                                                                    else {
//                                                                      self.complete(value);
//                                                                    }
//                                                                  },
//                                                                   executor);
//   }
//
//   struct glaze {
//     using type = filter<type_e::timer>;
//     static constexpr auto name{ "ipc::filter::timer" };
//     // clang-format off
//     static constexpr auto value{ glz::object("time_on", &type::time_on, "Rising edge settling delay",
//                                              "time_off", &type::time_off, "Falling edge settling delay") };
//     // clang-format on
//   };
// };

template <typename value_t, typename callback_t>
class filters {
public:
  filters(asio::io_context& ctx, callback_t&& callback) : ctx_{ ctx }, callback_{ callback } {
    if constexpr (std::is_same_v<bool, value_t>) {
      filters_.emplace_back(std::make_unique<filter<type_e::invert, value_t, asio::use_awaitable_t<>>>());
      filters_.emplace_back(std::make_unique<filter<type_e::timer, value_t, asio::use_awaitable_t<>>>());
      filters_.emplace_back(std::make_unique<filter<type_e::invert, value_t, asio::use_awaitable_t<>>>());
    }
  }

  void operator()(auto&& value) const {
    asio::co_spawn(
        ctx_,
        [this, copy = value] mutable -> asio::awaitable<value_t> {
          for (auto const& filter : filters_) {
            copy = co_await filter->async_process(copy, asio::use_awaitable);
          }
          // todo throw explicit runtime error if we should discard value, catch it and return silently
          co_return copy;
        },
        [this](std::exception_ptr exception_ptr, value_t return_val) {
          if (exception_ptr) {
            // todo log
            return;
          }
          std::invoke(callback_, return_val);
        });
  }

private:
  asio::io_context& ctx_;
  std::vector<std::unique_ptr<filter_interface<value_t, asio::use_awaitable_t<>>>> filters_{};
  callback_t callback_;
};

}  // namespace tfc::ipc::filter

template <>
struct glz::meta<tfc::ipc::filter::type_e> {
  using enum tfc::ipc::filter::type_e;
  static auto constexpr name{ "ipc::filter::type" };
  // clang-format off
  static auto constexpr value{ glz::enumerate("unkown", unknown,
                                              "invert", invert,
                                              "timer", timer,
                                              "offset", offset,
                                              "multiply", multiply,
                                              "calibrate_linear", calibrate_linear,
                                              "filter_out", filter_out,
                                              "median", median,
                                              "quantile", quantile,
                                              "sliding_window_moving_average", sliding_window_moving_average,
                                              "exponential_moving_average", exponential_moving_average,
                                              "throttle", throttle,
                                              "throttle_average", throttle_average,
                                              "delta", delta,
                                              "lambda", lambda) };
  // clang-format on
};
