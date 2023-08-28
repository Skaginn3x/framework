#pragma once
#include <concepts>
#include <exception>
#include <expected>
#include <vector>

#include <fmt/core.h>
#include <boost/asio/async_result.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <glaze/core/common.hpp>

#include <tfc/confman.hpp>

namespace tfc::ipc::filter {

namespace asio = boost::asio;

enum struct type_e : std::uint8_t {
  unknown = 0,
  invert,
  timer,
  // https://esphome.io/components/sensor/index.html#sensor-filters
  // todo: make the below filters
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

template <type_e type, typename value_t>
struct filter;

template <>
struct filter<type_e::invert, bool> {
  using value_t = bool;
  bool invert{ true };
  static constexpr type_e type{ type_e::invert };

  auto async_process(value_t const& value, auto&& completion_token) const {
    auto executor{ asio::get_associated_executor(completion_token) };
    return asio::async_compose<decltype(completion_token), void(std::expected<value_t, std::error_code>)>(
        [this, copy = value](auto& self) {
          if (invert) {
            self.complete(!copy);
          } else {
            self.complete(copy);
          }
        },
        completion_token, executor);
  }

  struct glaze {
    using type = filter<type_e::invert, value_t>;
    static constexpr auto name{ "tfc::ipc::filter::invert" };
    static constexpr auto value{
      glz::object("type", &type::type, "Type of filter", "invert", &type::invert, "Invert outputting value")
    };
  };
};

template <>
struct filter<type_e::timer, bool> {
  using value_t = bool;
  std::chrono::milliseconds time_on{ 5000 };
  std::chrono::milliseconds time_off{ 0 };
  static constexpr type_e type{ type_e::timer };

  filter() = default;
  filter(filter&&) noexcept = default;
  auto operator=(filter&&) noexcept -> filter& = default;
  filter(filter const& other) {
    this->time_on = other.time_on;
    this->time_off = other.time_off;
  }
  auto operator=(filter const& other) -> filter& {
    this->time_on = other.time_on;
    this->time_off = other.time_off;
    return *this;
  }

  auto async_process(value_t const& value, auto&& completion_token) const {
    auto exe{ asio::get_associated_executor(completion_token) };
    return asio::async_compose<decltype(completion_token), void(std::expected<value_t, std::error_code>)>(
        [this, copy = value, first_call = true](auto& self, std::error_code code = {}) mutable {
          if (code) {
            fmt::print("TODO DO serious business\n ");
            self.complete(std::unexpected(code));
            return;
          }
          if (first_call) {
            first_call = false;
            if (timer_) {
              // already waiting need to cancel and return
              timer_->cancel();  // this will make a recall to this very same lambda with error code
              timer_ = std::nullopt;
              return;
            }
            auto executor = asio::get_associated_executor(self);
            timer_ = asio::steady_timer{ executor };
            if (copy) {
              timer_->expires_from_now(time_on);
            } else {
              timer_->expires_from_now(time_off);
            }
            timer_->async_wait(std::move(self));
          } else {
            timer_ = std::nullopt;
            self.complete(copy);
          }
        },
        completion_token, exe);
  }

private:
  mutable std::optional<asio::steady_timer> timer_{ std::nullopt };

public:
  struct glaze {
    using type = filter<type_e::timer, value_t>;
    static constexpr auto name{ "tfc::ipc::filter::timer" };
    // clang-format off
    static constexpr auto value{ glz::object(
      "type", &type::type, "Type of filter",
      "time_on", &type::time_on, "Rising edge settling delay",
      "time_off", &type::time_off, "Falling edge settling delay"
    ) };
    // clang-format on
  };
};

template <typename value_t>
struct filter<type_e::filter_out, value_t> {
  value_t filter_out{};
  static constexpr type_e type{ type_e::filter_out };

  auto async_process(value_t const& value, auto&& completion_token) const {
    auto executor{ asio::get_associated_executor(completion_token) };
    return asio::async_compose<decltype(completion_token), void(std::expected<value_t, std::error_code>)>(
        [this, copy = value](auto& self) {
          // Todo should this filter be available for double?
          // clang-format off
          PRAGMA_CLANG_WARNING_PUSH_OFF(-Wfloat-equal)
          if (copy == filter_out) {
          PRAGMA_CLANG_WARNING_POP
            // clang-format on
            self.complete(std::unexpected(std::make_error_code(std::errc::bad_message)));
          } else {
            self.complete(copy);
          }
        },
        completion_token, executor);
  }

  struct glaze {
    using type = filter<type_e::filter_out, value_t>;
    static constexpr auto name{ "tfc::ipc::filter::filter_out" };
    static constexpr auto value{ glz::object("type",
                                             &type::type,
                                             "Type of filter",
                                             "filter_out",
                                             &type::filter_out,
                                             "If value is equivalent to this `filter_out` it will be ignored") };
  };
};

namespace exceptions {
class filter : public std::runtime_error {
public:
  explicit filter(std::error_code code) : std::runtime_error(code.message()) {}
};
}  // namespace exceptions
namespace detail {
template <typename value_t>
struct any_filter_decl;
template <>
struct any_filter_decl<bool> {
  using value_t = bool;
  using type =
      std::variant<filter<type_e::invert, value_t>, filter<type_e::timer, value_t>, filter<type_e::filter_out, value_t>>;
};
template <>
struct any_filter_decl<std::int64_t> {
  using value_t = std::int64_t;
  using type = std::variant<filter<type_e::filter_out, value_t>>;
};
template <>
struct any_filter_decl<std::uint64_t> {
  using value_t = std::uint64_t;
  using type = std::variant<filter<type_e::filter_out, value_t>>;
};
template <>
struct any_filter_decl<std::double_t> {
  using value_t = std::double_t;
  using type = std::variant<filter<type_e::filter_out, value_t>>;
};
template <>
struct any_filter_decl<std::string> {
  using value_t = std::string;
  using type = std::variant<filter<type_e::filter_out, value_t>>;
};
// json?
template <typename value_t>
using any_filter_decl_t = any_filter_decl<value_t>::type;
}  // namespace detail

template <typename value_t, typename callback_t>
class filters {
public:
  filters(asio::io_context& ctx, std::string_view name, callback_t&& callback)
      : ctx_{ ctx }, filters_{ ctx, fmt::format("{}/_filters_", name) }, callback_{ callback } {}

  void operator()(auto&& value) const {
    if (filters_->empty()) {
      std::invoke(callback_, value);
      return;
    }
    asio::co_spawn(
        ctx_,
        [this, copy = value] mutable -> asio::awaitable<value_t> {
          for (auto const& filter : filters_.value()) {
            auto return_value = co_await std::visit(
                [&copy](auto&& arg) -> auto { return arg.async_process(copy, asio::use_awaitable); }, filter);
            if (return_value) {
              copy = std::move(return_value.value());
            } else {
              throw exceptions::filter(return_value.error());
            }
          }
          co_return copy;
        },
        [this](std::exception_ptr const& exception_ptr, value_t return_val) {
          try {
            if (exception_ptr) {
              std::rethrow_exception(exception_ptr);
            }
          } catch (exceptions::filter const&) {
            fmt::print("Value has been forgotten\n");
            return;
          }
          std::invoke(callback_, return_val);
        });
  }

private:
  asio::io_context& ctx_;
  tfc::confman::config<std::vector<detail::any_filter_decl_t<value_t>>> filters_;
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
