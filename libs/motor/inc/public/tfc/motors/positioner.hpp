#pragma once

#include <array>
#include <cstddef>
#include <variant>
#include <string_view>

#include <fmt/format.h>
#include <boost/asio.hpp>
#include <glaze/core/common.hpp>

#include <tfc/logger.hpp>
#include <tfc/confman.hpp>

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
  positioner(asio::io_context& ctx, std::string_view name) : ctx_{ ctx }, name_{ name } {}
  struct positioner_config {
    struct tacho_not_used : std::monostate {
      struct glaze {
        static constexpr std::string_view name{ "Tachometer not used" };
      };
    };
    struct one_tacho_used {
      struct glaze {
        static constexpr std::string_view value{ "One tachometer" };
        static constexpr std::string_view name{ value };
      };
    };
    struct two_tacho_used {
      struct glaze {
        static constexpr std::string_view value{ "Two tachometers" };
        static constexpr std::string_view name{ value };
      };
    };
    std::variant<tacho_not_used, one_tacho_used, two_tacho_used> tacho{ tacho_not_used{} };
    struct glaze {
      using self = positioner_config;
      static constexpr std::string_view name{ "positioner_config" };
      static constexpr auto value{ glz::object(&self::tacho) };
    };
  };

private:
  asio ::io_context& ctx_;
  std::string name_;
  logger::logger logger_{ name_ };
  confman::config<positioner_config> config_{ ctx_, fmt::format("positioner_{}", name_) };
};

}  // namespace tfc::motor
