#pragma once
#include <concepts>
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
  offset,
  multiply,
  filter_out,
  // https://esphome.io/components/sensor/index.html#sensor-filters
  // todo: make the below filters
  calibrate_linear,  // https://github.com/esphome/esphome/blob/v1.20.4/esphome/components/sensor/__init__.py#L594
  median,
  quantile,
  sliding_window_moving_average,
  exponential_moving_average,
  throttle,
  throttle_average,
  delta,
  lambda,
};

template <type_e type, typename value_t, typename...>
struct filter;

/// \brief behaviour flip the state of boolean
template <>
struct filter<type_e::invert, bool> {
  using value_t = bool;
  static constexpr bool invert{ true };
  static constexpr type_e type{ type_e::invert };

  auto async_process(value_t const& value, auto&& completion_token) const {
    // todo can we get a compile error if executor is non-existent?
    auto exe = asio::get_associated_executor(completion_token);
    return asio::async_compose<decltype(completion_token), void(std::expected<value_t, std::error_code>)>(
        [copy = value](auto& self) { self.complete(!copy); }, completion_token, exe);
  }

  struct glaze {
    using type = filter<type_e::invert, value_t>;
    static constexpr auto name{ "tfc::ipc::filter::invert" };
    static constexpr auto value{
      glz::object("invert", &type::invert, tfc::json::schema{ .description = "Invert outputting value", .read_only = true })
    };
  };
};

/// \brief behaviour time on delay and or time off delay
/// \note IMPORTANT: delay changes take effect on next event
template <typename clock_type>  // default std::chrono::steady_clock
struct filter<type_e::timer, bool, clock_type> {
  using value_t = bool;
  std::chrono::milliseconds time_on{ 0 };
  std::chrono::milliseconds time_off{ 0 };
  static constexpr type_e type{ type_e::timer };

  filter() = default;
  // if the filter is moved everything is moved
  filter(filter&&) noexcept = default;
  auto operator=(filter&&) noexcept -> filter& = default;
  // if the filter is copied the data will be copied, the copied object will hold on to its timer `other`
  filter(filter const& other) {
    this->time_on = other.time_on;
    this->time_off = other.time_off;
  }
  auto operator=(filter const& other) -> filter& {
    this->time_on = other.time_on;
    this->time_off = other.time_off;
    return *this;
  }

  // async_process is const to not require making change to config object while processing the filter state
  auto async_process(value_t const& value, auto&& completion_token) const {
    // todo can we get a compile error if executor is non-existent?
    auto exe = asio::get_associated_executor(completion_token);
    return asio::async_compose<decltype(completion_token), void(std::expected<value_t, std::error_code>)>(
        [this, copy = value, first_call = true](auto& self, std::error_code code = {}) mutable {
          if (code) {
            // Callee will handle the error code
            self.complete(std::unexpected(code));
            return;
          }
          // second call meaning success, call owner and return
          if (!first_call) {
            timer_ = std::nullopt;
            self.complete(copy);
            return;
          }
          first_call = false;
          if (timer_) {
            // already waiting need to cancel and return
            timer_->cancel();  // this will make a recall to this very same lambda with error code
            timer_ = std::nullopt;
            return;
          }
          auto executor = asio::get_associated_executor(self);
          timer_ = asio::basic_waitable_timer<clock_type>{ executor };
          if (copy) {
            timer_->expires_after(time_on);
          } else {
            timer_->expires_after(time_off);
          }
          // moving self makes this callback be called once again when expiry is reached
          timer_->async_wait(std::move(self));
        },
        completion_token, exe);
  }

private:
  // mutable is required since async_process is const
  mutable std::optional<asio::basic_waitable_timer<clock_type>> timer_{ std::nullopt };

public:
  struct glaze {
    using type = filter<type_e::timer, value_t, clock_type>;
    static constexpr auto name{ "tfc::ipc::filter::timer" };
    // clang-format off
    static constexpr auto value{ glz::object(
      "time_on", &type::time_on, "Rising edge settling delay, applied on next event, NOT current one if already processing",
      "time_off", &type::time_off, "Falling edge settling delay, applied on next event, NOT current one if already processing"
    ) };
    // clang-format on
  };
};

template <typename value_t>
  requires requires { requires(std::integral<value_t> || std::floating_point<value_t>) && !std::same_as<value_t, bool>; }
struct filter<type_e::offset, value_t> {
  value_t offset{};
  static constexpr type_e type{ type_e::offset };

  auto async_process(value_t const& value, auto&& completion_token) const {
    // todo can we get a compile error if executor is non-existent?
    auto exe = asio::get_associated_executor(completion_token);
    return asio::async_compose<decltype(completion_token), void(std::expected<value_t, std::error_code>)>(
        [this, copy = value](auto& self) {
          self.complete(copy + offset);  //
        },
        completion_token, exe);
  }

  struct glaze {
    using type = filter<type_e::offset, value_t>;
    static constexpr auto name{ "tfc::ipc::filter::offset" };
    static constexpr auto value{ glz::object("offset", &type::offset, "Adds a constant value to each sensor value.") };
  };
};

template <typename value_t>
  requires requires { requires(std::integral<value_t> || std::floating_point<value_t>) && !std::same_as<value_t, bool>; }
struct filter<type_e::multiply, value_t> {
  value_t multiply{};
  static constexpr type_e type{ type_e::multiply };

  auto async_process(value_t const& value, auto&& completion_token) const {
    // todo can we get a compile error if executor is non-existent?
    auto exe = asio::get_associated_executor(completion_token);
    return asio::async_compose<decltype(completion_token), void(std::expected<value_t, std::error_code>)>(
        [this, copy = value](auto& self) {
          self.complete(copy * multiply);  //
        },
        completion_token, exe);
  }

  struct glaze {
    using type = filter<type_e::multiply, value_t>;
    static constexpr auto name{ "tfc::ipc::filter::multiply" };
    static constexpr auto value{ glz::object("multiply", &type::multiply, "Multiplies each value by a constant value.") };
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
    static constexpr auto value{
      glz::object("filter_out", &type::filter_out, "Filter out specific values to drop and forget")
    };
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
  using type = std::variant<filter<type_e::invert, value_t>,
                            filter<type_e::timer, value_t, std::chrono::steady_clock>,
                            filter<type_e::filter_out, value_t>>;
};
template <>
struct any_filter_decl<std::int64_t> {
  using value_t = std::int64_t;
  using type =
      std::variant<filter<type_e::filter_out, value_t>, filter<type_e::offset, value_t>, filter<type_e::multiply, value_t>>;
};
template <>
struct any_filter_decl<std::uint64_t> {
  using value_t = std::uint64_t;
  using type =
      std::variant<filter<type_e::filter_out, value_t>, filter<type_e::offset, value_t>, filter<type_e::multiply, value_t>>;
};
template <>
struct any_filter_decl<std::double_t> {
  using value_t = std::double_t;
  using type =
      std::variant<filter<type_e::filter_out, value_t>, filter<type_e::offset, value_t>, filter<type_e::multiply, value_t>>;
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
        [this, copy = value] mutable -> asio::awaitable<std::expected<value_t, std::error_code>> {
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
        [this](std::exception_ptr const& exception_ptr, std::expected<value_t, std::error_code> return_val) {
          if (exception_ptr) {
            std::rethrow_exception(exception_ptr);
          }
          if (return_val.has_value()) {
            std::invoke(callback_, return_val.value());
          } else {
            // todo log. I have now forgotten the original value/s
          }
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
  static auto constexpr value{ glz::enumerate("unknown", unknown,
                                              "invert", invert, "Invert outputting value",
                                              "timer", timer, "Timer on/off delay of boolean",
                                              "offset", offset, "Adds a constant value to each sensor value",
                                              "multiply", multiply, "Multiplies each value by a constant value",
                                              "filter_out", filter_out, "Filter out specific values to drop and forget",
                                              "calibrate_linear", calibrate_linear,
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
