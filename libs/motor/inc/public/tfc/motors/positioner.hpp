#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <mp-units/systems/si/unit_symbols.h>
#include <boost/asio.hpp>
#include <glaze/core/common.hpp>

#include <tfc/confman.hpp>
#include <tfc/ipc.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace tfc::motor::detail {
enum struct tacho_config_e : std::uint8_t { not_used = 0, one_tacho, two_tacho };
}

template <>
struct glz::meta<tfc::motor::detail::tacho_config_e> {
  static constexpr std::string_view name{ "tacho_config" };
  using enum tfc::motor::detail::tacho_config_e;
  static constexpr auto value{
    glz::enumerate("Not used", not_used, "One tachometer", one_tacho, "Two tachometers", two_tacho)
  };
};

namespace tfc::motor {

namespace asio = boost::asio;

namespace detail {

/// Tachometer with only one sensor, simply a counter upwards, direction cannot be determined
struct single_tachometer {
  single_tachometer() = default;
  explicit single_tachometer(std::function<void(std::int64_t)> position_update_callback) noexcept
      : position_update_callback_{ std::move(position_update_callback) } {}

  auto tacho_update(bool tacho_val) noexcept -> void {
    if (tacho_val) {
      position_ += 1;
      if (position_update_callback_) {
        (*position_update_callback_)(position_);
      }
    }
  }

  std::int64_t position_{};
  std::optional<std::function<void(std::int64_t)>> position_update_callback_;
};

/// Tachometer with two sensors, direction can be determined
struct double_tachometer {
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
  //	0	1	0	0	-1
  //	0	1	0	1	no movement
  //	0	1	1	1	+1
  //	1	0	0	0	+1
  //	1	0	1	0	no movement
  //	1	0	1	1	-1
  //	1	1	0	1	-1
  //	1	1	1	0	+1
  //	1	1	1	1	no movement

  double_tachometer() = default;
  explicit double_tachometer(std::function<void(std::int64_t)> position_update_callback) noexcept
      : position_update_callback_{ std::move(position_update_callback) } {}

  auto first_tacho_update(bool first_new_val) noexcept -> void {
    if (first_tacho) {
      if (second_tacho) {
        if (!first_new_val) {
          decrement();
        }
      } else {
        if (!first_new_val) {
          increment();
        }
      }
    } else {
      if (second_tacho) {
        if (first_new_val) {
          increment();
        }
      } else {
        if (first_new_val) {
          decrement();
        }
      }
    }

    first_tacho = first_new_val;
  }

  auto second_tacho_update(bool second_new_val) noexcept -> void {
    if (first_tacho) {
      if (second_tacho) {
        if (!second_new_val) {
          increment();
        }
      } else {
        if (second_new_val) {
          decrement();
        }
      }
    } else {
      if (!second_tacho) {
        if (second_new_val) {
          increment();
        }
      } else {
        if (!second_new_val) {
          decrement();
        }
      }
    }

    second_tacho = second_new_val;
  }

  auto increment() noexcept -> void {
    position_ += 1;
    if (position_update_callback_) {
      (*position_update_callback_)(position_);
    }
  }

  auto decrement() noexcept -> void {
    position_ -= 1;
    if (position_update_callback_) {
      (*position_update_callback_)(position_);
    }
  }

  bool first_tacho{};
  bool second_tacho{};
  std::int64_t position_{};
  std::optional<std::function<void(std::int64_t)>> position_update_callback_;
};

}  // namespace detail

using mp_units::quantity;
using mp_units::si::unit_symbols::mm;

struct config {
  detail::tacho_config_e tacho{ detail::tacho_config_e::not_used };
  // I would suggest default value as 2.54 cm (one inch) and ban 0 and negative values
  quantity<mm> displacement_per_pulse{};
  struct glaze {
    static constexpr std::string_view name{ "positioner_config" };
    static constexpr auto value{
      glz::object("tacho", &config::tacho, "displacement_per_tacho_pulse", &config::displacement_per_pulse)
    };
  };
};

template <class ipc_client_t = ipc_ruler::ipc_manager_client&, class config_t = tfc::confman::config<config>>
class positioner {
public:
  positioner(asio::io_context& ctx, ipc_client_t client, std::string_view name)
      : name_{ name }, ctx_{ ctx }, client_{ client } {
    switch (config_->tacho) {
      using enum detail::tacho_config_e;
      case not_used:
        break;
      case one_tacho: {
        single_tacho_.emplace([this](std::int64_t counts) { this->tick(counts); });
        tacho_a_.emplace(ctx_, client_, fmt::format("tacho_{}", name_),
                         "Tachometer input, usually induction sensor directed to rotational metal star or ring of screws.",
                         [this](bool val) { this->single_tacho_->tacho_update(val); });
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
  auto tick(std::int64_t tachometer_counts) -> void {
    absolute_position_ = config_->displacement_per_pulse * tachometer_counts;
    notify();
  }

  /// Check if any notifiers need to be notified
  auto notify() -> void {
    while (true) {
      if (notifications_.empty()) {
        return;
      }
      auto& notification = notifications_.front();

      if (absolute_position_ < notification.first) {
        break;
      }

      notification.second(absolute_position_);
      notifications_.erase(notifications_.begin());
      std::ranges::sort(notifications_.begin(), notifications_.end(), sort_notifications);
    }
  }

  static auto sort_notifications(std::pair<quantity<mm>, std::function<void(quantity<mm>)>>& a,
                                 std::pair<quantity<mm>, std::function<void(quantity<mm>)>>& b) -> bool {
    return a.first < b.first;
  }

  /// Place notifier on the list of notifiers, which will be notified when the positioner has moved the specified distance
  auto notify_after(quantity<mm> displacement, std::function<void(quantity<mm>)>& callback) -> void {
    notifications_.emplace_back(displacement + absolute_position_, callback);
    std::ranges::sort(notifications_.begin(), notifications_.end(), sort_notifications);
  }

  // Might be useful for homing sequences to grab the current position
  [[nodiscard]] auto position() const noexcept -> quantity<mm> { return absolute_position_; }
  // Please note the resolution
  [[nodiscard]] auto resolution() const noexcept -> quantity<mm> { return config_->displacement_per_pulse; }

private:
  std::string name_;
  asio::io_context& ctx_;
  ipc_client_t& client_;
  config_t config_{ ctx_, fmt::format("positioner_{}", name_) };
  std::optional<detail::single_tachometer> single_tacho_{};
  std::optional<detail::double_tachometer> double_tacho_{};
  std::optional<ipc::slot<ipc::details::type_bool, ipc_client_t&>> tacho_a_{};
  std::optional<ipc::slot<ipc::details::type_bool, ipc_client_t&>> tacho_b_{};
  // in terms of conveyors, mm resolution, this would overflow when you have gone 1800 trips to Pluto back and forth
  quantity<mm> absolute_position_{};
  std::vector<std::pair<quantity<mm>, std::function<void(quantity<mm>)>>> notifications_;
};

}  // namespace tfc::motor
