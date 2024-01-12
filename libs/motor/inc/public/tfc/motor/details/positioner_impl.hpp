#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <optional>
#include <string_view>
#include <variant>

#include <mp-units/systems/international/international.h>
#include <mp-units/systems/si/si.h>

#include <tfc/confman/observable.hpp>
#include <tfc/motor/errors.hpp>
#include <tfc/stx/constexpr_function.hpp>

namespace tfc::motor::positioner {

template <mp_units::Reference auto reference>
using deduce_velocity_t = mp_units::quantity<reference / mp_units::si::second, std::int64_t>;
using hertz_t = mp_units::quantity<mp_units::si::milli<mp_units::si::hertz>, std::int64_t>;
using tick_signature_t = void(std::int8_t, std::chrono::nanoseconds, std::chrono::nanoseconds, errors::err_enum);
using speedratio_t = mp_units::quantity<mp_units::percent, double>;  // todo extract to common place
namespace asio = boost::asio;

template <mp_units::Reference auto reference>
struct increment_config {
  using signed_dimension_t = mp_units::quantity<reference, std::int64_t>;
  static constexpr signed_dimension_t inch{ 1 * mp_units::international::inch };
  confman::observable<signed_dimension_t> displacement_per_increment{ signed_dimension_t{ inch } };
  confman::observable<std::chrono::microseconds> standard_deviation_threshold{ std::chrono::microseconds{ 100 } };
  bool operator==(increment_config const&) const noexcept = default;
};
template <mp_units::Reference auto reference>
struct tachometer_config : increment_config<reference> {};

template <mp_units::Reference auto reference>
struct encoder_config : increment_config<reference> {};

template <mp_units::Quantity velocity_t>
struct freq_config {
  confman::observable<velocity_t> velocity_at_50Hz{ 0 * velocity_t::reference };
  bool operator==(freq_config const&) const noexcept = default;
};

template <mp_units::Reference auto reference>
using position_mode_config = std::variant<std::monostate,
                                          tachometer_config<reference>,
                                          encoder_config<reference>,
                                          freq_config<deduce_velocity_t<reference>>>;

template <mp_units::Reference auto reference>
struct config {
  using unsigned_dimension_t = mp_units::quantity<reference, std::uint64_t>;
  confman::observable<position_mode_config<reference>> mode{ std::monostate{} };
  confman::observable<std::optional<unsigned_dimension_t>> needs_homing_after{ std::nullopt };
  confman::observable<std::optional<speedratio_t>> homing_travel_speed{
    std::nullopt
  };  // todo should this be outside this config?
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

template <typename clock_t = asio::steady_timer::time_point::clock, std::size_t circular_buffer_len = 128>
struct time_series_statistics {
  using duration_t = typename clock_t::duration;
  using time_point_t = typename clock_t::time_point;

  void update(time_point_t const& now) noexcept {
    // now let's calculate the average interval and variance
    last_interval_ = now - buffer_.front().time_point;
    auto const current_variance_increment{ std::pow(static_cast<double>(last_interval_.count()) - average_, 2) };

    auto const removed{ buffer_.emplace(now, last_interval_, current_variance_increment) };

    average_ += static_cast<double>(last_interval_.count() - removed.interval_duration.count()) / circular_buffer_len;
    // Note the variance is actually incorrect but it serves the purpose of detecting if the variance is increasing or not.
    // It is incorrect because it uses mean of values that have already been removed from the buffer to determine the
    // incremental variance of the older items in the buffer. I think it is impossible to calculate the correct variance in
    // constant time. For reference, see
    // https://math.stackexchange.com/questions/102978/incremental-computation-of-standard-deviation
    variance_ += (current_variance_increment - removed.variance_increment) / circular_buffer_len;
  }

  auto buffer() const noexcept -> auto const& { return buffer_; }

  auto average() const noexcept -> duration_t { return duration_t{ static_cast<typename duration_t::rep>(average_) }; }

  auto last_interval() const noexcept -> duration_t { return last_interval_; }

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
  duration_t last_interval_{};
  circular_buffer<event_storage, circular_buffer_len> buffer_{};
};

auto detect_deviation_from_average(auto interval, auto average) {
  auto err{ errors::err_enum::success };
  // check if interval is greater than 180% of average
  if (interval * 10 > average * 18) {
    err = errors::err_enum::positioning_missing_event;
  }
  return err;
}

template <typename manager_client_t = ipc_ruler::ipc_manager_client&,
          typename bool_slot_t = ipc::slot<ipc::details::type_bool, manager_client_t>,
          typename clock_t = asio::steady_timer::time_point::clock,
          std::size_t circular_buffer_len = 128>
struct tachometer {
  using duration_t = typename clock_t::duration;
  using time_point_t = typename clock_t::time_point;
  explicit tachometer(std::shared_ptr<sdbusplus::asio::connection> conn,
                      manager_client_t manager,
                      std::string_view name,
                      std::function<tick_signature_t>&& position_update_callback)
      : position_update_callback_{ std::move(position_update_callback) }, induction_sensor_{
          conn->get_io_context(), manager, fmt::format("tacho_{}", name),
          "Tachometer input, usually induction sensor directed to rotational metal star or plastic ring with metal bolts.",
          std::bind_front(&tachometer::update, this)
        } {}

  void update(bool first_new_val) noexcept {
    if (!first_new_val) {
      return;
    }
    auto now{ clock_t::now() };
    statistics_.update(now);
    auto constexpr increment{ 1 };
    position_ += increment;
    std::invoke(position_update_callback_, increment, statistics().average(), statistics().stddev(),
                detect_deviation_from_average(statistics_.last_interval(), statistics_.average()));
  }

  auto statistics() const noexcept -> auto const& { return statistics_; }

  std::int64_t position_{};  // todo now this is only for testing purposes
  time_series_statistics<clock_t, circular_buffer_len> statistics_{};
  std::function<tick_signature_t> position_update_callback_;
  bool_slot_t induction_sensor_;
};

template <typename manager_client_t = ipc_ruler::ipc_manager_client&, typename bool_slot_t = ipc::slot<ipc::details::type_bool, manager_client_t>,
          typename clock_t = asio::steady_timer::time_point::clock,
          std::size_t circular_buffer_len = 128>
struct encoder {
  explicit encoder(std::shared_ptr<sdbusplus::asio::connection> conn,
                   manager_client_t manager,
                   std::string_view name,
                   std::function<tick_signature_t>&& position_update_callback)
      : position_update_callback_{ std::move(position_update_callback) },
        sensor_a_{ conn->get_io_context(), manager, fmt::format("tacho_a_{}", name),
                   "First input of tachometer, with two sensors, usually induction sensor directed to rotational metal "
                   "star or plastic ring of metal bolts.",
                   std::bind_front(&encoder::first_tacho_update, this) },
        sensor_b_{ conn->get_io_context(), manager, fmt::format("tacho_b_{}", name),
                   "First input of tachometer, with two sensors, usually induction sensor directed to rotational metal "
                   "star or plastic ring of metal bolts.",
                   std::bind_front(&encoder::second_tacho_update, this) } {}

  struct storage {
    enum struct last_event_e : std::uint8_t { unknown = 0, first, second };
    bool first_tacho_state{};
    bool second_tacho_state{};
    last_event_e last_event{ last_event_e::unknown };
  };

  using last_event_t = typename storage::last_event_e;

  void first_tacho_update(bool first_new_val) noexcept {
    auto const increment{ first_new_val ? buffer_.front().second_tacho_state ? std::int8_t{ 1 } : std::int8_t{ -1 }
                          : buffer_.front().second_tacho_state ? std::int8_t{ -1 }
                                                               : std::int8_t{ 1 } };
    update(increment, first_new_val, buffer_.front().second_tacho_state, storage::last_event_e::first);
  }

  void second_tacho_update(bool second_new_val) noexcept {
    auto const increment{ second_new_val ? buffer_.front().first_tacho_state ? std::int8_t{ -1 } : std::int8_t{ 1 }
                          : buffer_.front().first_tacho_state ? std::int8_t{ 1 }
                                                              : std::int8_t{ -1 } };
    update(increment, buffer_.front().first_tacho_state, second_new_val, storage::last_event_e::second);
  }

  void update(std::int8_t increment, bool first, bool second, last_event_t event) noexcept {
    auto const now{ clock_t::now() };
    position_ += increment;
    fmt::println(stderr, "Encoder position: {}", position_);  // todo remove
    auto err{ detect_deviation_from_average(statistics_.last_interval(), statistics_.average()) };
    if (buffer_.front().last_event == event) {
      err = errors::err_enum::positioning_missing_event;
    }
    statistics_.update(now);
    buffer_.emplace(first, second, event);
    std::invoke(position_update_callback_, increment, statistics_.average(), statistics_.stddev(), err);
  }

  std::int64_t position_{};  // todo now this is only for testing purposes, need to refactor tests
  circular_buffer<storage, circular_buffer_len> buffer_{};
  time_series_statistics<clock_t, circular_buffer_len> statistics_{};
  std::function<tick_signature_t> position_update_callback_;
  bool_slot_t sensor_a_;
  bool_slot_t sensor_b_;
};

template <mp_units::Quantity dimension_t,
          typename clock_t = asio::steady_timer::time_point::clock,
          std::size_t circular_buffer_len = 128>
struct frequency {
  static constexpr auto reference{ dimension_t::reference };
  using velocity_t = mp_units::quantity<reference / mp_units::si::second, std::int64_t>;
  using time_point_t = typename clock_t::time_point;
  using callback_t = std::function<void(dimension_t, velocity_t)>;
  static constexpr hertz_t reference_frequency_50Hz{ 50 * mp_units::si::hertz };

  explicit frequency(velocity_t velocity_at_50Hz, callback_t&& position_update_callback) noexcept
      : velocity_at_50Hz_{ velocity_at_50Hz }, position_update_callback_{ std::move(position_update_callback) } {}

  void update_velocity_at_50Hz(velocity_t velocity_at_50Hz) noexcept { velocity_at_50Hz_ = velocity_at_50Hz; }

  void freq_update(hertz_t hertz) {
    // Update our traveled distance from the new frequency
    // Calculate the distance the motor would have traveled on the old speed
    auto now = clock_t::now();
    mp_units::quantity<mp_units::si::nano<mp_units::si::second>, int64_t> const intvl_current{ now - time_point_ };
    time_point_ = now;
    auto const velocity{ velocity_at_50Hz_ * last_measured_ / reference_frequency_50Hz };
    auto const displacement = mp_units::value_cast<dimension_t::reference>(intvl_current * velocity_at_50Hz_ *
                                                                           last_measured_ / reference_frequency_50Hz);
    last_measured_ = hertz;
    std::invoke(position_update_callback_, displacement, velocity);
  }

  velocity_t velocity_at_50Hz_{};
  hertz_t last_measured_{ 0 * hertz_t::reference };
  callback_t position_update_callback_;
  time_point_t time_point_{ clock_t::now() };
};

template <typename unsigned_t>
constexpr auto make_between_callable(unsigned_t before, unsigned_t now, bool forward = true)
    -> stx::function<bool(unsigned_t)> {
  if (forward) {
    // detect overflow
    if (now < before) {
      return [before, now](unsigned_t x) -> bool { return x >= before || x <= now; };
    }
    // this is the default increment case
    return [before, now](unsigned_t x) -> bool { return x >= before && x <= now; };
  }
  // detect underflow
  if (before < now) {
    return [before, now](unsigned_t x) -> bool { return x >= now || x <= before; };
  }
  // this is the default decrement case
  return [before, now](unsigned_t x) -> bool { return x >= now && x <= before; };
}

}  // namespace detail

}  // namespace tfc::motor::positioner

template <mp_units::Reference auto reference>
struct glz::meta<tfc::motor::positioner::increment_config<reference>> {
  static constexpr std::string_view name{ "tfc::motor::positioner::increment_config" };
  using self = tfc::motor::positioner::increment_config<reference>;
  // clang-format off
  static constexpr auto value{
    glz::object(
      "displacement_per_increment", &self::displacement_per_increment, tfc::json::schema{
        .description = "Displacement per increment\n"
                       "Mode: tachometer, displacement per pulse or distance between two teeths\n"
                       "Mode: encoder, displacement per edge, distance between two teeths divided by 4",
        .default_value = self::inch.numerical_value_ref_in(reference),
        .minimum = 1UL,
      },
      "standard_deviation_threshold", &self::standard_deviation_threshold, tfc::json::schema{
        .description = "Standard deviation between increments, used to determine if signal is stable",
        .default_value = 100UL,
        .minimum = 1UL,
      }
    )
  };
  // clang-format on
};
template <mp_units::Reference auto reference>
struct glz::meta<tfc::motor::positioner::tachometer_config<reference>> {
  static constexpr std::string_view name{ "Tachometer" };
  static constexpr auto value{ glz::meta<tfc::motor::positioner::increment_config<reference>>::value };
};
template <mp_units::Reference auto reference>
struct glz::meta<tfc::motor::positioner::encoder_config<reference>> {
  static constexpr std::string_view name{ "Encoder" };
  static constexpr auto value{ glz::meta<tfc::motor::positioner::increment_config<reference>>::value };
};
template <mp_units::Quantity velocity_t>
struct glz::meta<tfc::motor::positioner::freq_config<velocity_t>> {
  static constexpr std::string_view name{ "Frequency" };
  using self = tfc::motor::positioner::freq_config<velocity_t>;
  // clang-format off
  static constexpr auto value{
    glz::object(
      "velocity_at_50Hz", &self::velocity_at_50Hz, tfc::json::schema{
        .description = "Velocity at 50Hz",
        .default_value = 0UL,
      }
    )
  };
  // clang-format on
};
template <mp_units::Reference auto reference>
struct glz::meta<tfc::motor::positioner::position_mode_config<reference>> {
  static constexpr std::string_view tag{ "mode" };
};

template <mp_units::Reference auto reference>
struct glz::meta<tfc::motor::positioner::config<reference>> {
  static constexpr std::string_view name{ "tfc::motor::positioner::config" };
  using self = tfc::motor::positioner::config<reference>;
  // clang-format off
  static constexpr auto value{
    glz::object(
      "mode", &self::mode, tfc::json::schema{
        .description = "Selection of positioning mode",
      },
      "needs_homing_after", &self::needs_homing_after, tfc::json::schema{
        .description = "Only used in when homing reference is used, "
                       "Require homing after the specified displacement, try 1 kilometre."
        // .default_value = 1000000000UL, // todo use explicit type, needs to handle at least µm/s and µL/s
      },
      "homing_travel_speed", &self::homing_travel_speed, tfc::json::schema{
        .description = "Speedratio (-100% to 100%) used when homing, sign indicates direction",
        .minimum = -100L,
        .maximum = 100L,
      }
    )
  };
  // clang-format on
};
