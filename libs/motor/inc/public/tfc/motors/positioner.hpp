#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>
#include <type_traits>  // required by mp-units

#include <fmt/format.h>
#include <mp-units/systems/si/si.h>
#include <boost/asio.hpp>
#include <glaze/core/common.hpp>

#include <tfc/confman.hpp>
#include <tfc/ipc.hpp>
#include <tfc/logger.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace tfc::motor::detail {
enum struct tacho_config : std::uint8_t { not_used = 0, one_tacho, two_tacho };
}

template <>
struct glz::meta<tfc::motor::detail::tacho_config> {
  static constexpr std::string_view name{ "tacho_config" };
  using enum tfc::motor::detail::tacho_config;
  static constexpr auto value{
    glz::enumerate("Not used", not_used, "One tachometer", one_tacho, "Two tachometers", two_tacho)
  };
};

namespace tfc::motor {

namespace asio = boost::asio;

namespace detail {
template <typename storage_t, std::size_t len>
struct circular_buffer {
  circular_buffer() = default;
  template <typename... args_t>
  constexpr auto emplace(args_t&&... args) {
    front_ = insert_pos_;
    std::construct_at(insert_pos_, std::forward<args_t>(args)...);
    std::advance(insert_pos_, 1);
    if (insert_pos_ == std::end(buffer_)) {
      insert_pos_ = std::begin(buffer_);
    }
  }
  constexpr auto front() noexcept -> storage_t& { return *front_; }
  constexpr auto front() const noexcept -> storage_t const& { return *front_; }

  std::array<storage_t, len> buffer_{};
  // front is invalid when there has no item been inserted yet, but should not matter much
  typename std::array<storage_t, len>::iterator front_{ std::begin(buffer_) };
  typename std::array<storage_t, len>::iterator insert_pos_{ std::begin(buffer_) };
};

template <typename time_point_t = asio::steady_timer::time_point, std::size_t circular_buffer_len = 1024>
struct single_tachometer {
  explicit single_tachometer(std::function<void(std::int64_t)> position_update_callback) noexcept
      : position_update_callback_{ std::move(position_update_callback) } {}

  void first_tacho_update(bool first_new_val) noexcept {
    if (first_new_val) {
      increment();
    }

    auto now{ time_point_t::clock::now() };
    buffer_.emplace(first_new_val, now);
  }

  auto increment() noexcept -> void {
    position_ += 1;
    position_update_callback_(position_);
  }

  struct storage {
    bool tacho_state{};
    time_point_t time_point{};
  };

  circular_buffer<storage, circular_buffer_len> buffer_{};
  std::int64_t position_{};
  std::function<void(std::int64_t)> position_update_callback_ = [](std::int64_t) {};
};

template <typename time_point_t = asio::steady_timer::time_point, std::size_t circular_buffer_len = 1024>
struct double_tachometer {
  explicit double_tachometer(std::function<void(std::int64_t)> position_update_callback) noexcept
      : position_update_callback_{ std::move(position_update_callback) } {}

  // https://github.com/PaulStoffregen/Encoder/blob/master/Encoder.h#L303
  //                           _______         _______
  //               Pin1 ______|       |_______|       |______ Pin1
  // negative <---         _______         _______         __      --> positive
  //               Pin2 __|       |_______|       |_______|   Pin2

  //	new 	new	    old 	old
  //	pin2	pin1	pin2	pin1	Result
  //	----	----	----	----	------
  //	0	0	0	0	no movement
  //	0	0	0	1	+1
  //	0	0	1	0	-1
  //	0	0	1	1	+2  (assume pin1 edges only)
  //	0	1	0	0	-1
  //	0	1	0	1	no movement
  //	0	1	1	0	-2  (assume pin1 edges only)
  //	0	1	1	1	+1
  //	1	0	0	0	+1
  //	1	0	0	1	-2  (assume pin1 edges only)
  //	1	0	1	0	no movement
  //	1	0	1	1	-1
  //	1	1	0	0	+2  (assume pin1 edges only)
  //	1	1	0	1	-1
  //	1	1	1	0	+1
  //	1	1	1	1	no movement
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
    auto now{ time_point_t::clock::now() };
    buffer_.emplace(first_new_val, buffer_.front().second_tacho_state, now);
    update();
  }

  void second_tacho_update(bool second_new_val) noexcept {
    auto now{ time_point_t::clock::now() };
    buffer_.emplace(buffer_.front().first_tacho_state, second_new_val, now);
    update();
  }

  auto update() noexcept -> void {
    uint8_t s = state & 3;

    if (buffer_.front().first_tacho_state) {
      s |= 4;
    }

    if (buffer_.front().second_tacho_state) {
      s |= 8;
    }

    // clang-format off
    switch (s) {
      case 0: case 5: case 10: case 15:
        break;
      case 1: case 7: case 8: case 14:
        increment(1);
        break;
      case 2: case 4: case 11: case 13:
        decrement(1);
        break;
      case 3: case 12:
        increment(2);
        break;
      default:
        decrement(2);
        break;
    }
    // clang-format on
    state = (s >> 2);
  }

  auto increment(std::int64_t pos) noexcept -> void {
    position_ += pos;
    position_update_callback_(position_);
  }

  auto decrement(std::int64_t pos) noexcept -> void {
    position_ -= pos;
    position_update_callback_(position_);
  }

  struct storage {
    bool first_tacho_state{};
    bool second_tacho_state{};
    time_point_t time_point{};
  };

  circular_buffer<storage, circular_buffer_len> buffer_{};
  std::int64_t position_{};
  std::function<void(std::int64_t)> position_update_callback_ = [](std::int64_t) {};
  uint8_t state = 0;
};

}  // namespace detail

template <mp_units::Quantity dimension_t = mp_units::quantity<mp_units::si::milli<mp_units::si::metre>, std::int64_t>>
struct config {
  detail::tacho_config tacho{ detail::tacho_config::not_used };
  dimension_t displacement_per_pulse{ 1 * mp_units::si::unit_symbols::mm };
  struct glaze {
    static constexpr std::string_view name{ "positioner_config" };
    static constexpr auto value{
      glz::object("tacho", &config::tacho, "displacement_per_tacho_pulse", &config::displacement_per_pulse)
    };
  };
};

template <mp_units::Quantity dimension_t = mp_units::quantity<mp_units::si::milli<mp_units::si::metre>, std::int64_t>,
          typename ipc_client_t = tfc::ipc_ruler::ipc_manager_client&,
          typename config_t = tfc::confman::config<config<dimension_t>>>
class positioner {
public:
  static constexpr auto reference{ dimension_t::reference };

  explicit positioner(asio::io_context& ctx, ipc_client_t client, std::string_view name)
      : name_{ name }, ctx_{ ctx }, client_{ client } {
    switch (config_->tacho) {
      using enum detail::tacho_config;
      case not_used:
        break;
      case one_tacho: {
        single_tacho_.emplace([this](std::int64_t counts) { this->tick(counts); });
        tacho_a_.emplace(ctx_, client_, fmt::format("tacho_{}", name_),
                         "Tachometer input, usually induction sensor directed to rotational metal star og ring of screws.",
                         [this](bool val) { this->single_tacho_->first_tacho_update(val); });
        break;
      }
      case two_tacho: {
        double_tacho_.emplace([this](std::int64_t counts) { this->tick(counts); });
        tacho_a_.emplace(
            ctx_, client_, fmt::format("tacho_a_{}", name_),
            "First input of tachometer, with two sensors, usually induction sensor directed to rotational metal "
            "star og ring of screws.",
            [this](bool val) { this->double_tacho_->first_tacho_update(val); });
        tacho_b_.emplace(ctx_, client_, fmt::format("tacho_b_{}", name_),
                         "Second input of tachometer, with two sensors, usually induction sensor directed to rotational "
                         "metal star og ring of screws.",
                         [this](bool val) { this->double_tacho_->second_tacho_update(val); });
        break;
      }
    }
  }

  /// Tachometer should call this function every time it increments/decrements its position
  void tick(std::int64_t tachometer_counts) {
    absolute_position_ = config_->displacement_per_pulse * tachometer_counts;
    notify();
  }

  /// Check if any notifiers need to be notified
  /// If their position is equal to or further than the current absolute position then notify
  auto notify() -> void {
    while (true) {
      if (notifications_.empty()) {
        return;
      }
      auto& notification = notifications_.front();

      if (absolute_position_ < notification.absolute_notification_position_) {
        break;
      }

      notification.completion_handler();
      notifications_.erase(notifications_.begin());
      std::ranges::sort(notifications_.begin(), notifications_.end(), sort_notifications);
    }
  }

  /// Maintain information about when notifiers should be notified and their respective callbacks
  struct notification {
    dimension_t absolute_notification_position_{};
    std::function<void()> completion_handler;

    notification(dimension_t pos, std::function<void()> handler)
        : absolute_notification_position_{ pos }, completion_handler{ std::move(handler) } {}
  };

  auto notify_after(dimension_t displacement, std::function<void()> callback) -> void {
    notifications_.emplace_back(displacement + absolute_position_, std::move(callback));
    /// Sort by absolute position so that we can easily check if we need to notify
    std::sort(notifications_.begin(), notifications_.end(), sort_notifications);
  }

  static auto sort_notifications(const notification& lhs, const notification& rhs) {
    return lhs.absolute_notification_position_ < rhs.absolute_notification_position_;
  }

  // Might be useful for homing sequences to grab the current position
  // Please note the resolution
  [[nodiscard]] dimension_t position() const noexcept { return absolute_position_; }
  [[nodiscard]] dimension_t resolution() const noexcept { return config_->displacement_per_pulse; }

private:
  std::string name_;
  asio::io_context& ctx_;
  ipc_client_t client_;
  logger::logger logger_{ name_ };
  config_t config_{ ctx_, fmt::format("positioner_{}", name_) };
  std::optional<detail::single_tachometer<>> single_tacho_{};
  std::optional<detail::double_tachometer<>> double_tacho_{};
  std::optional<ipc::slot<ipc::details::type_bool, ipc_client_t>> tacho_a_{};
  std::optional<ipc::slot<ipc::details::type_bool, ipc_client_t>> tacho_b_{};
  /// In terms of conveyors, mm resolution, this would overflow when you have gone 1800 trips to Pluto back and forth
  dimension_t absolute_position_{};
  std::vector<notification> notifications_{};
};

}  // namespace tfc::motor
