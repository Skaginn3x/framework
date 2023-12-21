#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <functional>
#include <string_view>
#include <type_traits>  // required by mp-units
#include <variant>
#include <vector>

#include <fmt/format.h>
#include <mp-units/chrono.h>
#include <mp-units/math.h>
#include <mp-units/systems/international/international.h>
#include <mp-units/systems/si/si.h>

#include <boost/asio.hpp>
#include <glaze/core/common.hpp>

#include <tfc/confman.hpp>
#include <tfc/ipc.hpp>
#include <tfc/logger.hpp>
#include <tfc/stx/concepts.hpp>
#include <tfc/utils/asio_condition_variable.hpp>
#include <tfc/utils/json_schema.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace tfc::motor::detail {
enum struct position_mode_e : std::uint8_t { not_used = 0, tachometer, encoder, frequency };
}

template <>
struct glz::meta<tfc::motor::detail::position_mode_e> {
  static constexpr std::string_view name{ "tacho_config" };
  using enum tfc::motor::detail::position_mode_e;
  static constexpr auto value{
    glz::enumerate("Not used", not_used, "Tachometer", tachometer, "Encoder", encoder, "Frequency", frequency)
  };
};

namespace tfc::motor {

namespace asio = boost::asio;
using position_t = mp_units::quantity<mp_units::si::micro<mp_units::si::metre>, std::int64_t>;
using tick_signature_t = void(std::int64_t, std::chrono::nanoseconds, std::chrono::nanoseconds);

// to be added to `notify_at` , the completion token
enum struct position_error_code_e : std::uint8_t {
  no_error = 0,
  unstable,       // could be one event missing per cycle or similar
  missing_event,  // missing event from encoder
  unknown,        // cause not explicitly known
};

namespace detail {
template <typename storage_t, std::size_t len>
struct circular_buffer {
  circular_buffer() = default;

  /// \param args arguments to forward to constructor of storage_t
  /// \return removed item, the oldest item
  constexpr auto emplace(auto&&... args) -> storage_t {
    storage_t removed_item{ std::move(*insert_pos_) };
    front_ = insert_pos_;
    std::construct_at(insert_pos_, std::forward<decltype(args)>(args)...);
    std::advance(insert_pos_, 1);
    if (insert_pos_ == std::end(buffer_)) {
      insert_pos_ = std::begin(buffer_);
    }
    return std::move(removed_item);
  }
  /// \return reference to most recently inserted item
  constexpr auto front() noexcept -> storage_t& { return *front_; }
  /// \return const reference to most recently inserted item
  constexpr auto front() const noexcept -> storage_t const& { return *front_; }
  /// \return reference to oldest inserted item
  constexpr auto back() noexcept -> storage_t& { return *insert_pos_; }
  /// \return const reference to oldest inserted item
  constexpr auto back() const noexcept -> storage_t const& { return *insert_pos_; }

  std::array<storage_t, len> buffer_{};
  // front is invalid when there has no item been inserted yet, but should not matter much
  typename std::array<storage_t, len>::iterator front_{ std::begin(buffer_) };
  typename std::array<storage_t, len>::iterator insert_pos_{ std::begin(buffer_) + 1 };  // this is front + 1
};

template <typename clock_t = asio::steady_timer::time_point::clock, std::size_t circular_buffer_len = 1024>
struct time_series_statistics {
  using duration_t = typename clock_t::duration;
  using time_point_t = typename clock_t::time_point;

  void update(time_point_t const& now) noexcept {
    // now let's calculate the average interval and variance
    auto const intvl_current{ now - buffer_.front().time_point };
    auto const current_variance_increment{ std::pow(static_cast<double>(intvl_current.count()) - average_, 2) };

    auto const removed{ buffer_.emplace(now, intvl_current, current_variance_increment) };

    average_ += static_cast<double>(intvl_current.count() - removed.interval_duration.count()) / circular_buffer_len;
    // Note the variance is actually incorrect but it serves the purpose of detecting if the variance is increasing or not.
    // It is incorrect because it uses mean of values that have already been removed from the buffer to determine the
    // incremental variance of the older items in the buffer. I think it is impossible to calculate the correct variance in
    // constant time. For reference, see
    // https://math.stackexchange.com/questions/102978/incremental-computation-of-standard-deviation
    variance_ += (current_variance_increment - removed.variance_increment) / circular_buffer_len;
  }

  auto buffer() const noexcept -> auto const& { return buffer_; }

  auto average() const noexcept -> duration_t { return duration_t{ static_cast<typename duration_t::rep>(average_) }; }

  auto stddev() const noexcept -> duration_t {
    // assert(variance_ >= 0, "Variance is squared so it should be positive.");
    return duration_t{ static_cast<typename duration_t::rep>(std::sqrt(variance_)) };
  }

  struct event_storage {
    time_point_t time_point{};
    duration_t interval_duration{};
    double variance_increment{};
  };
  double average_{};
  double variance_{};
  circular_buffer<event_storage, circular_buffer_len> buffer_{};
};

template <typename bool_slot_t = ipc::bool_slot,
          typename clock_t = asio::steady_timer::time_point::clock,
          std::size_t circular_buffer_len = 1024>
struct tachometer {
  using duration_t = typename clock_t::duration;
  using time_point_t = typename clock_t::time_point;
  explicit tachometer(asio::io_context& ctx,
                      ipc_ruler::ipc_manager_client& client,
                      std::string_view name,
                      std::function<tick_signature_t>&& position_update_callback)
      : position_update_callback_{ std::move(position_update_callback) }, induction_sensor_{
          ctx, client, fmt::format("tacho_{}", name),
          "Tachometer input, usually induction sensor directed to rotational metal star or plastic ring with metal bolts.",
          std::bind_front(&tachometer::update, this)
        } {}

  void update(bool first_new_val) noexcept {
    if (!first_new_val) {
      return;
    }
    auto now{ clock_t::now() };
    statistics_.update(now);
    std::invoke(position_update_callback_, ++position_, statistics().average(), statistics().stddev());
  }

  auto statistics() const noexcept -> auto const& { return statistics_; }

  std::int64_t position_{};
  time_series_statistics<clock_t, circular_buffer_len> statistics_{};
  std::function<tick_signature_t> position_update_callback_{ [](auto, auto, auto) {} };
  bool_slot_t induction_sensor_;
};

template <typename bool_slot_t = ipc::bool_slot,
          typename clock_t = asio::steady_timer::time_point::clock,
          std::size_t circular_buffer_len = 1024>
struct encoder {
  explicit encoder(asio::io_context& ctx,
                   ipc_ruler::ipc_manager_client& client,
                   std::string_view name,
                   std::function<tick_signature_t>&& position_update_callback)
      : position_update_callback_{ std::move(position_update_callback) },
        sensor_a_{ ctx, client, fmt::format("tacho_a_{}", name),
                   "First input of tachometer, with two sensors, usually induction sensor directed to rotational metal "
                   "star or plastic ring of metal bolts.",
                   std::bind_front(&encoder::first_tacho_update, this) },
        sensor_b_{ ctx, client, fmt::format("tacho_b_{}", name),
                   "First input of tachometer, with two sensors, usually induction sensor directed to rotational metal "
                   "star or plastic ring of metal bolts.",
                   std::bind_front(&encoder::second_tacho_update, this) } {}

  // https://github.com/PaulStoffregen/Encoder/blob/master/Encoder.h#L303
  //                           _______         _______
  //               Pin1 ______|       |_______|       |______ Pin1
  // negative <---         _______         _______         __      --> positive
  //               Pin2 __|       |_______|       |_______|   Pin2

  //	new 	new	    old 	old
  //	pin2	pin1	pin2	pin1	Result
  //	----	----	----	----	------
  //	0	0	0	0	no movement
  //	0	1	0	1	no movement
  //	1	0	1	0	no movement
  //	1	1	1	1	no movement
  //	1	0	0	0	+1
  //	0	0	0	1	+1
  //	0	1	1	1	+1
  //	1	1	1	0	+1
  //	0	1	0	0	-1
  //	0	0	1	0	-1
  //	1	0	1	1	-1
  //	1	1	0	1	-1
  //	0	0	1	1	+2  (assume pin1 edges only)
  //	0	1	1	0	-2  (assume pin1 edges only)
  //	1	0	0	1	-2  (assume pin1 edges only)
  //	1	1	0	0	+2  (assume pin1 edges only)
  /*
          // Simple, easy-to-read "documentation" version :-)
          //
          void update(void) {
                  uint8_t s = state & 3;
                  if (digitalRead(pin1)) s |= 4;
                  if (digitalRead(pin2)) s |= 8;
                  switch (s) {
                          case 0: case 5: case 10: case 15:
                                  break;
                          case 1: case 7: case 8: case 14:
                                  position++; break;
                          case 2: case 4: case 11: case 13:
                                  position--; break;
                          case 3: case 12:
                                  position += 2; break;
                          default:
                                  position -= 2; break;
                  }
                  state = (s >> 2);
          }
  */

  void first_tacho_update(bool first_new_val) noexcept {
    update({ .new_first = first_new_val,
             .new_second = buffer_.front().second_tacho_state,
             .old_first = buffer_.front().first_tacho_state,
             .old_second = buffer_.front().second_tacho_state });
  }

  void second_tacho_update(bool second_new_val) noexcept {
    update({ .new_first = buffer_.front().first_tacho_state,
             .new_second = second_new_val,
             .old_first = buffer_.front().first_tacho_state,
             .old_second = buffer_.front().second_tacho_state });
  }

  struct update_params {
    bool new_first : 1 {};
    bool new_second : 1 {};
    bool old_first : 1 {};
    bool old_second : 1 {};
    std::uint8_t not_used : 4 {};
    constexpr auto operator==(update_params const& other) const noexcept -> bool = default;
    constexpr explicit(false) operator std::uint8_t() const noexcept {
      // The below is not constexpr yet, let's keep this comment here until it is
      // return std::bit_cast<std::uint8_t>(*this);
      return static_cast<std::uint8_t>((old_second << 3) | (old_first << 2) | (new_second << 1) | new_first);
    }
  };

  constexpr auto update(update_params params) noexcept -> void {
    auto const now{ clock_t::now() };
    auto const increment{ update_impl(params) };  // todo error cases
    if (increment != 0) {
      buffer_.emplace(bool{ params.new_first }, bool{ params.new_second });
      statistics_.update(now);
      position_ += increment;
      std::invoke(position_update_callback_, position_, statistics_.average(), statistics_.stddev());
    }
  }

  static constexpr auto update_impl(update_params params) noexcept -> std::int64_t {
    // please note that this assumes the duty cycle being 50%
    // if the duty cycle is not 50% then the position will be off by maximum of the skewed cycle
    // example, if 10% ON and 90% OFF, then the position will be off by 40% of the resolution (90% - 50%) or (50% - 10%)
    // todo, determine duty cycle and increase resolution with timer between triggers
    // clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wimplicit-fallthrough) // todo why do I need this?
    // clang-format on
    switch (params) {
      [[unlikely]] case update_params{}:
      [[unlikely]] case update_params{ .new_second = true, .old_second = true }:
      [[unlikely]] case update_params{ .new_first = true, .old_first = true }:
      [[unlikely]] case update_params{ .new_first = true, .new_second = true, .old_first = true, .old_second = true }:
        return 0;
      case update_params{ .new_second = true }:
      case update_params{ .old_first = true }:
      case update_params{ .new_first = true, .old_first = true, .old_second = true }:
      case update_params{ .new_first = true, .new_second = true, .old_second = true }:
        return 1;
      case update_params{ .new_first = true }:
      case update_params{ .old_second = true }:
      case update_params{ .new_second = true, .old_first = true, .old_second = true }:
      case update_params{ .new_first = true, .new_second = true, .old_first = true }:
        return -1;
      [[unlikely]] case update_params{ .new_first = true, .new_second = true }:
      [[unlikely]] case update_params{ .old_first = true, .old_second = true }:
        return 2;  // this is impossible in the using codepath, because new states are distinct between a and b
      [[unlikely]] case update_params{ .new_second = true, .old_first = true }:
      [[unlikely]] case update_params{ .new_first = true, .old_second = true }:
        return -2;  // this is impossible in the using codepath, because new states are distinct between a and b
                    // clang-format off
      [[unlikely]] default:  // theoretically should never be called
        return 0;
                    // clang-format on
    }
    PRAGMA_CLANG_WARNING_POP
  }

  struct storage {
    bool first_tacho_state{};
    bool second_tacho_state{};
  };

  std::int64_t position_{};
  circular_buffer<storage, circular_buffer_len> buffer_{};
  time_series_statistics<clock_t, circular_buffer_len> statistics_{};
  std::function<tick_signature_t> position_update_callback_{ [](auto, auto, auto) {} };
  bool_slot_t sensor_a_;
  bool_slot_t sensor_b_;
};

template <mp_units::Quantity dimension_t,
          typename time_point_t = asio::steady_timer::time_point,
          std::size_t circular_buffer_len = 1024>
struct frequency {
  explicit frequency(std::function<void(dimension_t)>&& position_update_callback) noexcept
      : position_update_callback_{ std::move(position_update_callback) } {}

  void on_freq(mp_units::quantity<mp_units::si::milli<mp_units::si::hertz>, std::int64_t>) {
    // todo
  }
  std::function<void(dimension_t)> position_update_callback_{ [](dimension_t) {} };
};

}  // namespace detail

template <mp_units::Quantity dimension_t = position_t, template <typename> typename confman_t = confman::config>
class positioner {
public:
  static constexpr auto reference{ dimension_t::reference };
  using velocity_t = mp_units::quantity<reference / mp_units::si::second, std::int64_t>;

  struct config {
    detail::position_mode_e mode{ detail::position_mode_e::not_used };
    static constexpr dimension_t inch{ 1 * mp_units::international::inch };
    dimension_t displacement_per_increment{ inch };
    struct glaze {
      static constexpr std::string_view name{ "positioner_config" };
      // clang-format off
      static constexpr auto value{
        glz::object(
          "mode", &config::mode,
          "displacement_per_increment", &config::displacement_per_increment, json::schema{
            .description = "Displacement per increment\n"
                           "Mode: tachometer, displacement per pulse or distance between two teeths\n"
                           "Mode: encoder, displacement per edge, distance between two teeths divided by 4",
            .default_value = inch.numerical_value_ref_in(dimension_t::reference),
            .minimum = 1,
          }
        )
      };
      // clang-format on
    };
  };

  /// \param ctx boost asio context
  /// \param client ipc client
  /// \param name to concatenate to slot names, example atv320_12 where 12 is slave id
  positioner(asio::io_context& ctx, ipc_ruler::ipc_manager_client& client, std::string_view name)
      : positioner(ctx, client, name, config{}) {}

  /// \param ctx boost asio context
  /// \param client ipc client
  /// \param name to concatenate to slot names, example atv320_12 where 12 is slave id
  /// \param default_value configuration default, useful for special cases and testing
  positioner(asio::io_context& ctx, ipc_ruler::ipc_manager_client& client, std::string_view name, config&& default_value)
    : name_{ name }, ctx_{ ctx }, config_{ ctx_, fmt::format("positioner_{}", name_), std::move(default_value) } {
    switch (config_->mode) {
      using enum detail::position_mode_e;
      case not_used:
        break;
      case tachometer: {
        impl_.template emplace<detail::tachometer<>>(ctx_, client, name_, std::bind_front(&positioner::tick, this));
        break;
      }
      case encoder: {
        impl_.template emplace<detail::encoder<>>(ctx_, client, name_, std::bind_front(&positioner::tick, this));
        break;
      }
      case frequency: {
        impl_.template emplace<detail::frequency<dimension_t>>(std::bind_front(&positioner::increment_position, this));
        break;
      }
    }
  }

  /// \param position absolute position
  /// \param token completion token to trigger when get passed the given position
  auto notify_at(dimension_t const& position, asio::completion_token_for<void(std::error_code)> auto&& token)
      -> decltype(auto) {
    using return_t = typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type;
    auto const sort_by_nearest{ [abs_pos = absolute_position_](notification const& lhs, notification const& rhs) {
      auto const lhs_diff{ mp_units::abs(abs_pos - lhs.absolute_notification_position_) };
      auto const rhs_diff{ mp_units::abs(abs_pos - rhs.absolute_notification_position_) };
      return lhs_diff < rhs_diff;  // returns true if left hand side is nearer to current position
    } };
    using cv = tfc::asio::condition_variable<asio::any_io_executor>;
    auto& notifications{ position < absolute_position_ ? notifications_backward_ : notifications_forward_ };
    notifications.emplace_back(position, cv{ ctx_.get_executor() });
    if constexpr (std::same_as<return_t, void>) {
      notifications.back().cv_.async_wait(std::forward<decltype(token)>(token));
      std::ranges::sort(notifications, sort_by_nearest);
      return;
    } else {
      return_t result{ notifications.back().cv_.async_wait(std::forward<decltype(token)>(token)) };
      std::ranges::sort(notifications, sort_by_nearest);
      return std::move(result);
    }
  }

  /// \param displacement from current position
  /// \param token completion token to trigger when get passed the given displacement
  auto notify_after(dimension_t const& displacement, asio::completion_token_for<void(std::error_code)> auto&& token)
      -> decltype(auto) {
    return notify_at(displacement + absolute_position_, std::forward<decltype(token)>(token));
  }

  /// \param displacement from home
  /// \param token completion token to trigger when get passed the given displacement
  auto notify_from_home(dimension_t const& displacement, asio::completion_token_for<void(std::error_code)> auto&& token)
      -> decltype(auto) {
    return notify_at(displacement + home_, std::forward<decltype(token)>(token));
  }

  /// \return Current position since last restart
  [[nodiscard]] auto position() const noexcept -> dimension_t { return absolute_position_; }

  /// \return Resolution of the absolute position
  [[nodiscard]] auto resolution() const noexcept -> dimension_t { return config_->displacement_per_increment; }

  /// \return Current velocity
  [[nodiscard]] auto velocity() const noexcept -> velocity_t { return velocity_; }

  /// \brief home is used in method notify_from_home to determine the notification position
  /// \relates notify_from_home
  /// \param new_home_position store this position as home
  void home(dimension_t const& new_home_position) noexcept { home_ = new_home_position; }

  void increment_position(dimension_t const& increment) {
    auto const old_position{ absolute_position_ };
    absolute_position_ += increment;
    notify_if_applicable(old_position);
  }

  void tick(std::int64_t tachometer_counts,
            [[maybe_unused]] std::chrono::nanoseconds const& average,
            [[maybe_unused]] std::chrono::nanoseconds const& stddev) {
    auto const old_position{ absolute_position_ };
    absolute_position_ = config_->displacement_per_increment * tachometer_counts;
    // Important the resolution below needs to match
    static constexpr auto nanometer_reference{ mp_units::si::nano<mp_units::si::metre> };
    static constexpr auto nanosec_reference{ mp_units::si::nano<mp_units::si::second> };
    if (average.count() > 0) {
      auto const displacement{ (absolute_position_ - old_position).in(nanometer_reference) };
      velocity_ = displacement / (average.count() * nanosec_reference);
    } else {
      velocity_ = 0 * (reference / nanosec_reference);
    }
    notify_if_applicable(old_position);
  }

private:
  void notify_if_applicable(dimension_t const& old_position) {
    constexpr auto iterate_notifications{ [](auto& notifications, auto const& current_position, auto const& last_position) {
      for (auto it{ std::begin(notifications) }; it != std::end(notifications); ++it) {
        auto const pos{ it->absolute_notification_position_ };
        // check if pos is in range of last position and current position
        if (pos >= std::min(last_position, current_position) && pos <= std::max(last_position, current_position)) {
          it->cv_.notify_all();
          // it would be nice to pop_front here, but we need the event loop to be able to call the notification before we
          // erase it todo discuss, move iterator to discard vector Condition variable can expose whether there is
          // outstanding completion call
          asio::post([&notifications, it] {  // Think it is safe to capture by reference here
            notifications.erase(it);
          });
        } else {
          return;
        }
      }
    } };
    iterate_notifications(notifications_forward_, absolute_position_, old_position);
    iterate_notifications(notifications_backward_, old_position, absolute_position_);
    // todo make a timer and call the notification when timer expires
    // remember to determine duty cycle for encoder
  }

  struct notification {
    dimension_t absolute_notification_position_{};
    tfc::asio::condition_variable<asio::any_io_executor> cv_;
    auto operator<=>(notification const& other) const noexcept {
      return absolute_notification_position_ <=> other.absolute_notification_position_;
    }
  };

  dimension_t absolute_position_{};
  dimension_t home_{};
  velocity_t velocity_{};
  std::string name_{};
  asio::io_context& ctx_;
  logger::logger logger_{ name_ };
  confman_t<config> config_;
  std::variant<detail::frequency<dimension_t>, detail::tachometer<>, detail::encoder<>> impl_{
    detail::frequency<dimension_t>{ std::bind_front(&positioner::increment_position, this) }
  };
  // todo overflow
  // in terms of conveyors, mm resolution, this would overflow when you have gone 1.8 trips to Pluto back and forth
  std::vector<notification> notifications_forward_{};
  std::vector<notification> notifications_backward_{};
};

}  // namespace tfc::motor
