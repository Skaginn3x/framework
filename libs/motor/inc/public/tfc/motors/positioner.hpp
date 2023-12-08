#pragma once

#include <array>
#include <cstddef>
#include <string_view>

#include <fmt/format.h>
#include <boost/asio.hpp>
#include <glaze/core/common.hpp>

#include <tfc/confman.hpp>
#include <tfc/ipc.hpp>
#include <tfc/logger.hpp>

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
struct tachometer {
  void first_tacho_update(bool state) noexcept {
    auto now{ time_point_t::clock::now() };
    buffer_.emplace(state, buffer_.front().second_tacho_state, now);
  }
  void second_tacho_update(bool state) noexcept {
    auto now{ time_point_t::clock::now() };
    buffer_.emplace(buffer_.front().first_tacho_state, state, now);
  }

  struct storage {
    bool first_tacho_state{};
    bool second_tacho_state{};
    time_point_t time_point{};
  };
  circular_buffer<storage, circular_buffer_len> buffer_{};
};

}  // namespace detail

class positioner {
public:
  struct config {
    detail::tacho_config tacho{ detail::tacho_config::not_used };
    struct glaze {
      static constexpr std::string_view name{ "positioner_config" };
      static constexpr auto value{ glz::object("tacho", &config::tacho) };
    };
  };

  positioner(asio::io_context& ctx, ipc_ruler::ipc_manager_client& client, std::string_view name)
      : name_{ name }, ctx_{ ctx }, client_{ client } {
    switch (config_->tacho) {
      using enum detail::tacho_config;
      case not_used:
        break;
      case one_tacho: {
        tacho_.emplace();
        tacho_a.emplace(ctx_, client_, fmt::format("tacho_{}", name_),
                        "Tachometer input, usually induction sensor directed to rotational metal star og ring of screws.",
                        [this](bool val) { this->tacho_->first_tacho_update(val); });
        break;
      }
      case two_tacho: {
        tacho_.emplace();
        tacho_a.emplace(ctx_, client_, fmt::format("tacho_a_{}", name_),
                        "First input of tachometer, with two sensors, usually induction sensor directed to rotational metal "
                        "star og ring of screws.",
                        [this](bool val) { this->tacho_->first_tacho_update(val); });
        tacho_b.emplace(ctx_, client_, fmt::format("tacho_b_{}", name_),
                        "Second input of tachometer, with two sensors, usually induction sensor directed to rotational "
                        "metal star og ring of screws.",
                        [this](bool val) { this->tacho_->second_tacho_update(val); });
        break;
      }
    }
  }

private:
  std::string name_;
  asio::io_context& ctx_;
  ipc_ruler::ipc_manager_client& client_;
  logger::logger logger_{ name_ };
  confman::config<config> config_{ ctx_, fmt::format("positioner_{}", name_) };
  std::optional<detail::tachometer<>> tacho_{};
  std::optional<ipc::bool_slot> tacho_a{};
  std::optional<ipc::bool_slot> tacho_b{};
};

}  // namespace tfc::motor
