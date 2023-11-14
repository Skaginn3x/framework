#pragma once
#include <concepts>
#include <expected>
#include <vector>

#include <boost/asio/async_result.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <glaze/core/common.hpp>

#include <tfc/confman.hpp>
#include <tfc/dbus/sdbusplus_fwd.hpp>
#include <tfc/stx/glaze_meta.hpp>
#include <tfc/utils/pragmas.hpp>

namespace tfc::ipc::filter {

namespace asio = boost::asio;

enum struct filter_e : std::uint8_t {
  unknown = 0,
  new_state,
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
  tfc_item,  // according to item json schema see ipc/item.hpp, TODO: implement
};

template <filter_e type, typename value_t, typename...>
struct filter;

/// \brief behaviour flip the state of boolean
template <>
struct filter<filter_e::invert, bool> {
  static constexpr bool const_value{ true };

  auto async_process(bool&& value, auto&& completion_token) const {
    // todo can we get a compile error if executor is non-existent?
    auto exe = asio::get_associated_executor(completion_token);
    return asio::async_compose<decltype(completion_token), void(std::expected<bool, std::error_code>)>(
        [copy = value](auto& self) { self.complete(!copy); }, completion_token, exe);
  }

  struct glaze {
    using type = filter<filter_e::invert, bool>;
    static constexpr std::string_view name{ "tfc::ipc::filter::invert" };
    static constexpr auto value{ glz::object("invert", &type::const_value) };
  };
};

/// \brief behaviour time on delay and or time off delay
/// \note IMPORTANT: delay changes take effect on next event
template <typename clock_type>  // example std::chrono::steady_clock
struct filter<filter_e::timer, bool, clock_type> {
  std::chrono::milliseconds time_on{ 0 };
  std::chrono::milliseconds time_off{ 0 };
  static constexpr filter_e type{ filter_e::timer };

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
  auto async_process(bool&& value, auto&& completion_token) const {
    // todo can we get a compile error if executor is non-existent?
    auto exe = asio::get_associated_executor(completion_token);
    return asio::async_compose<decltype(completion_token), void(std::expected<bool, std::error_code>)>(
        [this, copy = value, first_call = true](auto& self, std::error_code code = {}) mutable {
          if (code) {
            // Base case for a timer that was not able to complete(cancelled).
            // IE a state change occurred while waiting or class deconstructed.
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
          timer_->expires_after(copy ? time_on : time_off);
          // moving self makes this callback be called once again when expiry is reached or timer is cancelled
          timer_->async_wait(std::move(self));
        },
        completion_token, exe);
  }

private:
  // mutable is required since async_process is const
  mutable std::optional<asio::basic_waitable_timer<clock_type>> timer_{ std::nullopt };

public:
  struct glaze {
    using type = filter<filter_e::timer, bool, clock_type>;
    static constexpr std::string_view name{ "tfc::ipc::filter::timer" };
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
struct filter<filter_e::offset, value_t> {
  value_t offset{};
  static constexpr filter_e type{ filter_e::offset };

  auto async_process(value_t&& value, auto&& completion_token) const {
    // todo can we get a compile error if executor is non-existent?
    auto exe = asio::get_associated_executor(completion_token);
    return asio::async_compose<decltype(completion_token), void(std::expected<value_t, std::error_code>)>(
        [this, copy = value](auto& self) {
          self.complete(copy + offset);  //
        },
        completion_token, exe);
  }

  struct glaze {
    using type = filter<filter_e::offset, value_t>;
    static constexpr std::string_view name{ "tfc::ipc::filter::offset" };
    static constexpr auto value{ glz::object("offset", &type::offset, "Adds a constant value to each sensor value.") };
  };
};

template <typename value_t>
  requires requires { requires(std::integral<value_t> || std::floating_point<value_t>) && !std::same_as<value_t, bool>; }
struct filter<filter_e::multiply, value_t> {
  value_t multiply{};
  static constexpr filter_e type{ filter_e::multiply };

  auto async_process(value_t&& value, auto&& completion_token) const {
    // todo can we get a compile error if executor is non-existent?
    auto exe = asio::get_associated_executor(completion_token);
    return asio::async_compose<decltype(completion_token), void(std::expected<value_t, std::error_code>)>(
        [this, copy = value](auto& self) {
          self.complete(copy * multiply);  //
        },
        completion_token, exe);
  }

  struct glaze {
    using type = filter<filter_e::multiply, value_t>;
    static constexpr std::string_view name{ "tfc::ipc::filter::multiply" };
    static constexpr auto value{ glz::object("multiply", &type::multiply, "Multiplies each value by a constant value.") };
  };
};

template <typename value_t>
struct filter<filter_e::filter_out, value_t> {
  value_t filter_out{};
  static constexpr filter_e type{ filter_e::filter_out };

  auto async_process(value_t&& value, auto&& completion_token) const {
    auto executor{ asio::get_associated_executor(completion_token) };
    return asio::async_compose<decltype(completion_token), void(std::expected<value_t, std::error_code>)>(
        [this, moved_value = std::move(value)](auto& self) {
          // Todo should this filter be available for double?
          // clang-format off
          PRAGMA_CLANG_WARNING_PUSH_OFF(-Wfloat-equal)
          if (moved_value == filter_out) {
          PRAGMA_CLANG_WARNING_POP
            // clang-format on
            self.complete(std::unexpected(std::make_error_code(std::errc::bad_message)));
          } else {
            self.complete(moved_value);
          }
        },
        completion_token, executor);
  }

  struct glaze {
    using type = filter<filter_e::filter_out, value_t>;
    static constexpr std::string_view name{ "tfc::ipc::filter::filter_out" };
    static constexpr auto value{
      glz::object("filter_out", &type::filter_out, "Filter out specific values to drop and forget")
    };
  };
};

namespace detail {
template <typename value_t>
struct any_filter_decl;
template <>
struct any_filter_decl<bool> {
  using value_t = bool;
  using type = std::variant<filter<filter_e::invert, value_t>, filter<filter_e::timer, value_t, std::chrono::steady_clock>>;
};
template <>
struct any_filter_decl<std::int64_t> {
  using value_t = std::int64_t;
  using type = std::
      variant<filter<filter_e::filter_out, value_t>, filter<filter_e::offset, value_t>, filter<filter_e::multiply, value_t>>;
};
template <>
struct any_filter_decl<std::uint64_t> {
  using value_t = std::uint64_t;
  using type = std::
      variant<filter<filter_e::filter_out, value_t>, filter<filter_e::offset, value_t>, filter<filter_e::multiply, value_t>>;
};
template <>
struct any_filter_decl<std::double_t> {
  using value_t = std::double_t;
  using type = std::
      variant<filter<filter_e::filter_out, value_t>, filter<filter_e::offset, value_t>, filter<filter_e::multiply, value_t>>;
};
template <>
struct any_filter_decl<std::string> {
  using value_t = std::string;
  using type = std::variant<filter<filter_e::filter_out, value_t>>;
};
// json?
template <typename value_t>
using any_filter_decl_t = any_filter_decl<value_t>::type;
}  // namespace detail

template <typename value_t, tfc::stx::invocable<value_t> callback_t>
class filters {
public:
  using config_t = std::vector<detail::any_filter_decl_t<value_t>>;

  filters(std::shared_ptr<sdbusplus::asio::dbus_interface> interface, tfc::stx::invocable<value_t> auto&& callback)
      : ctx_{ interface->connection()->get_io_context() }, filters_{ interface, "Filter" },
        callback_{ std::forward<decltype(callback)>(callback) } {}

  /// \brief changes internal last_value state when filters have been processed
  void operator()(auto&& value)
    requires std::same_as<std::remove_cvref_t<decltype(value)>, value_t>
  {
    if (filters_->empty()) {
      last_value_ = std::forward<decltype(value)>(value);
      std::invoke(callback_, last_value_.value());
      return;
    }
    std::expected<value_t, std::error_code> return_value{ std::forward<decltype(value)>(value) };
    asio::co_spawn(
        ctx_,
        [this, return_val = std::move(return_value)] mutable -> asio::awaitable<std::expected<value_t, std::error_code>> {
          for (auto const& filter : filters_.value()) {
            // move the value into the filter and the filter will return the value modified or not
            return_val = co_await std::visit(
                [return_v = std::move(return_val)](auto&& arg) mutable -> auto {  // mutable to move return_value
                  return arg.async_process(std::move(return_v.value()), asio::use_awaitable);  //
                },
                filter);
            if (!return_val.has_value()) {
              // The filter has erased the existence of inputted value, exit the coroutine and forget that this happened
              co_return std::move(return_val);
            }
          }
          co_return std::move(return_val);
        },
        [this](std::exception_ptr const& exception_ptr, std::expected<value_t, std::error_code>&& return_val) {
          if (exception_ptr) {
            std::rethrow_exception(exception_ptr);
          }
          if (return_val.has_value()) {
            last_value_ = std::move(return_val.value());
            std::invoke(callback_, last_value_.value());
          } else {
            // I have now forgotten the original value/s
          }
        });
  }

  /// \brief bypass filters and set value directly
  void set(auto&& value)
    requires std::same_as<std::remove_cvref_t<decltype(value)>, value_t>
  {
    last_value_ = std::forward<decltype(value)>(value);
    std::invoke(callback_, last_value_.value());
  }

  [[nodiscard]] auto value() const noexcept -> std::optional<value_t> const& { return last_value_; }

private:
  asio::io_context& ctx_;
  tfc::confman::config<config_t> filters_;
  callback_t callback_;
  std::optional<value_t> last_value_{};
};

}  // namespace tfc::ipc::filter

template <>
struct glz::meta<tfc::ipc::filter::filter_e> {
  using enum tfc::ipc::filter::filter_e;
  static std::string_view constexpr name{ "ipc::filter::filter_e" };
  // clang-format off
  static auto constexpr value{ glz::enumerate("unknown", unknown,
                                              "new_state", new_state, "Call caller only on new state",
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
