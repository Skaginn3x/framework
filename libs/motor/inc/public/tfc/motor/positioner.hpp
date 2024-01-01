#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
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
#include <tfc/ipc/details/dbus_client_iface.hpp>
#include <tfc/logger.hpp>
#include <tfc/stx/concepts.hpp>
#include <tfc/utils/asio_condition_variable.hpp>
#include <tfc/utils/json_schema.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

#include <tfc/motor/details/positioner_impl.hpp>

namespace tfc::motor::positioner {

template <mp_units::PrefixableUnit auto unit_v = mp_units::si::metre,
          template <typename, typename, typename> typename confman_t = confman::config>
class positioner {
public:
  static constexpr auto unit{ unit_v };
  static constexpr auto reference{ mp_units::si::nano<unit> };
  // using unsigned integer to get detirmistic overflow/underflow
  using absolute_position_t = mp_units::quantity<reference, std::uint64_t>;
  // easier to use signed integer to represent displacement
  using displacement_t = mp_units::quantity<reference, std::int64_t>;
  using velocity_t = mp_units::quantity<reference / mp_units::si::second, std::int64_t>;
  using config_t = config<reference>;

  /// \param ctx boost asio context
  /// \param client ipc client
  /// \param name to concatenate to slot names, example atv320_12 where 12 is slave id
  positioner(asio::io_context& ctx, ipc_ruler::ipc_manager_client& client, std::string_view name)
      : positioner(ctx, client, name, {}) {}

  /// \param ctx boost asio context
  /// \param client ipc client
  /// \param name to concatenate to slot names, example atv320_12 where 12 is slave id
  /// \param default_value configuration default, useful for special cases and testing
  positioner(asio::io_context& ctx, ipc_ruler::ipc_manager_client& client, std::string_view name, config_t&& default_value)
      : name_{ name }, ctx_{ ctx }, client_{ client },
        config_{ ctx_, fmt::format("positioner_{}", name_), std::move(default_value) } {
    config_->mode.observe(std::bind_front(&positioner::construct_implementation, this));
    construct_implementation(config_->mode, {});
    config_->needs_homing_after.observe(
        [this](auto const& new_v, auto const& old_v) { missing_home_ = !old_v.has_value() && new_v.has_value(); });
  }

  positioner(positioner const&) = delete;
  auto operator=(positioner const&) -> positioner& = delete;
  positioner(positioner&&) noexcept = default;
  auto operator=(positioner&&) noexcept -> positioner& = default;
  ~positioner() = default;

  /// \param position absolute position
  /// \param token completion token to trigger when get passed the given position
  /// \todo implicitly allow motor::errors::err_enum as param in token
  auto notify_at(absolute_position_t position, asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    using cv = tfc::asio::condition_variable<asio::any_io_executor>;
    auto new_notification = std::make_shared<notification>(position, cv{ ctx_.get_executor() });
    notifications_.emplace_back(new_notification);
    return asio::async_compose<decltype(token), void(std::error_code)>(
        [first_call = true, new_notification](auto& self, std::error_code err = {}) mutable {
          if (first_call) {
            first_call = false;
            new_notification->cv_.async_wait(std::move(self));
            return;
          }
          if (err) {
            self.complete(err);
            return;
          }
          self.complete(motor_error(new_notification->err_));
        },
        token, ctx_);
  }

  /// \param displacement from current position
  /// \param token completion token to trigger when get passed the given displacement
  auto notify_after(displacement_t displacement, asio::completion_token_for<void(std::error_code)> auto&& token)
      -> decltype(auto) {
    return notify_at(displacement + absolute_position_, std::forward<decltype(token)>(token));
  }

  /// \param displacement from home
  /// \param token completion token to trigger when get passed the given displacement
  auto notify_from_home(displacement_t displacement, asio::completion_token_for<void(std::error_code)> auto&& token)
      -> decltype(auto) {
    return notify_at(displacement + home_, std::forward<decltype(token)>(token));
  }

  /// \return Current position since last restart
  [[nodiscard]] auto position() const noexcept -> absolute_position_t { return absolute_position_; }

  /// \return Resolution of the absolute position
  [[nodiscard]] auto resolution() const noexcept -> displacement_t { return config_->displacement_per_increment; }

  /// \return Current velocity
  [[nodiscard]] auto velocity() const noexcept -> velocity_t { return velocity_; }

  /// \brief home is used in method notify_from_home to determine the notification position
  /// \relates notify_from_home
  /// \param new_home_position store this position as home
  void home(absolute_position_t new_home_position) noexcept {
    home_ = new_home_position;
    travel_since_homed_ = 0 * reference;
    missing_home_ = false;
    if (last_error_ == errors::err_enum::motor_missing_home_reference) {
      last_error_ = errors::err_enum::success;
    }
  }

  /// \brief home is used in method notify_from_home to determine the notification position
  /// \relates notify_from_home
  /// Will store current position as home
  void home() noexcept { return home(position()); }

  /// \brief return current error state
  /// \return error code
  [[nodiscard]] auto error() const noexcept -> errors::err_enum {
    using enum errors::err_enum;
    // special case when we have not yet homed and we recovered from some other error
    // it might be beneficial to refactor error to being bitset struct of error bool flags
    if (config_->needs_homing_after->has_value() && last_error_ == success && missing_home_) {
      return motor_missing_home_reference;
    }
    return last_error_;
  }

  void freq_update(hertz_t hertz) {
    std::visit(
        [hertz]<typename impl_t>(impl_t& impl) {
          if constexpr (std::same_as<std::remove_cvref_t<impl_t>, detail::frequency<displacement_t>>) {
            impl.freq_update(hertz);
          }
        },
        impl_);
  }

  void increment_position(displacement_t increment, velocity_t velocity = 0 * velocity_t::reference) {
    auto const old_position{ absolute_position_ };
    auto const forward{ increment > 0 * decltype(increment)::reference };
    absolute_position_ += increment;
    velocity_ = velocity;
    if (needs_homing(increment)) {
      if (last_error_ == errors::err_enum::success) {
        last_error_ = errors::err_enum::motor_missing_home_reference;
      }
    }
    notify_if_applicable(old_position, forward);
  }

  void tick(std::int64_t tachometer_counts,
            std::chrono::nanoseconds average,
            std::chrono::nanoseconds stddev,
            errors::err_enum err) {
    auto const new_position{ displacement_per_increment_ * tachometer_counts };
    auto const old_position{ absolute_position_ };
    auto const increment{ new_position - old_position };  // todo underflow
    stddev_ = stddev;
    last_error_ = err;
    if (stddev_ >= standard_deviation_threshold_) {
      last_error_ = errors::err_enum::positioning_unstable;
    }
    // Important the resolution below needs to match
    static constexpr auto nanometer_reference{ mp_units::si::nano<mp_units::si::metre> };
    static constexpr auto nanosec_reference{ mp_units::si::nano<mp_units::si::second> };
    velocity_t velocity{};
    if (average > std::chrono::seconds{ 0 }) {
      velocity = increment.in(nanometer_reference) / (average.count() * nanosec_reference);
    } else {
      velocity = 0 * velocity_t::reference;
    }
    increment_position(increment, velocity);
  }

private:
  auto needs_homing(displacement_t increment) -> bool {
    if (!config_->needs_homing_after->has_value()) {
      return false;
    }
    auto const old_travel_since_homed{ travel_since_homed_ };
    travel_since_homed_ += mp_units::abs(increment);
    auto const pos{ config_->needs_homing_after->value() };
    if (pos >= old_travel_since_homed && pos <= travel_since_homed_) {
      missing_home_ = true;
    }
    return missing_home_;
  }

  auto construct_implementation(position_mode_config<reference> const& mode_to_construct,
                                [[maybe_unused]] position_mode_config<reference> const& old_mode) noexcept {
    using tachometer_config_t = tachometer_config<reference>;
    using encoder_config_t = encoder_config<reference>;
    using freq_config_t = freq_config<deduce_velocity_t<reference>>;
    std::visit(
        [this]<typename mode_t>(mode_t const& mode) {
          using mode_raw_t = std::remove_cvref_t<mode_t>;
          if constexpr (std::same_as<mode_raw_t, tachometer_config_t>) {
            impl_.template emplace<detail::tachometer<>>(ctx_, client_, name_, std::bind_front(&positioner::tick, this));

            displacement_per_increment_ = mode.displacement_per_increment.value();
            standard_deviation_threshold_ = mode.standard_deviation_threshold.value();
            // mode.displacement_per_increment.observe(
            //     [this](mp_units::QuantityOf<reference> auto const& new_v, auto&) { displacement_per_increment_ = new_v;
            //     });
            mode.standard_deviation_threshold.observe(
                [this](std::chrono::microseconds new_v, auto) { standard_deviation_threshold_ = new_v; });
          } else if constexpr (std::same_as<mode_raw_t, encoder_config_t>) {
            impl_.template emplace<detail::encoder<>>(ctx_, client_, name_, std::bind_front(&positioner::tick, this));

            // todo duplicate
            displacement_per_increment_ = mode.displacement_per_increment.value();
            standard_deviation_threshold_ = mode.standard_deviation_threshold.value();
            // mode.displacement_per_increment.observe(
            //     [this](mp_units::QuantityOf<reference> auto const& new_v, auto&) { displacement_per_increment_ = new_v;
            //     });
            mode.standard_deviation_threshold.observe(
                [this](std::chrono::microseconds new_v, auto) { standard_deviation_threshold_ = new_v; });
          } else if constexpr (std::same_as<mode_raw_t, freq_config_t>) {
            impl_.template emplace<detail::frequency<displacement_t>>(
                mode.velocity_at_50Hz, std::bind_front(&positioner::increment_position, this));

            // mode.velocity_at_50Hz.observe(
            //     [](auto const& new_v, auto&) {
            //       std::visit([new_v]<typename impl_t>(impl_t& impl) {
            //         if constexpr (std::same_as<std::remove_cvref_t<impl_t>, detail::frequency<displacement_t>>) {
            //           impl.update_velocity_at_50Hz(new_v);
            //         }
            //       });
            //     },
            //     impl_);
          } else {
            impl_ = std::monostate{};
          }
        },
        mode_to_construct);
  }

  void notify_if_applicable(absolute_position_t old_position, bool forward) {
    auto const comparator{ detail::make_between_callable(old_position, absolute_position_, forward) };
    std::erase_if(notifications_, [this, &comparator](auto& n) {
      auto const pos{ n->abs_notify_pos_ };
      if (comparator(pos)) {
        n->err_ = last_error_;
        n->cv_.notify_all();
        return true;
      }
      return false;
    });
  }

  struct notification {
    absolute_position_t abs_notify_pos_{};
    tfc::asio::condition_variable<asio::any_io_executor> cv_;
    errors::err_enum err_{ errors::err_enum::success };
    auto operator<=>(notification const& other) const noexcept { return abs_notify_pos_ <=> other.abs_notify_pos_; }
  };

  errors::err_enum last_error_{ errors::err_enum::success };
  absolute_position_t absolute_position_{};
  absolute_position_t travel_since_homed_{};
  absolute_position_t home_{};
  displacement_t displacement_per_increment_{};
  velocity_t velocity_{};
  std::chrono::nanoseconds stddev_{};
  std::chrono::nanoseconds standard_deviation_threshold_{};
  std::string name_{};
  asio::io_context& ctx_;
  ipc_ruler::ipc_manager_client& client_;
  logger::logger logger_{ name_ };
  confman_t<config_t, confman::file_storage<config_t>, confman::detail::config_dbus_client> config_;
  std::variant<std::monostate, detail::frequency<displacement_t>, detail::tachometer<>, detail::encoder<>> impl_{};
  // todo overflow
  // in terms of conveyors, µm resolution, this would overflow when you have gone 1.8 trips to Pluto back and forth
  std::vector<std::shared_ptr<notification>> notifications_{};

  bool missing_home_{ config_->needs_homing_after->has_value() };
};

}  // namespace tfc::motor::positioner
