#pragma once

namespace tfc::motor {

namespace detail {
template <typename storage_t, std::size_t len>
struct circular_buffer {
  circular_buffer() = default;
  template <typename... args_t>
  constexpr auto emplace(args_t&&... args) {
    std::construct_at(pos_, std::forward<args_t>(args)...);
    if (++pos_ == std::end(buffer_)) {
      pos_ = std::begin(buffer_);
    }
  }

  // todo use static_vector, https://github.com/arturbac/small_vectors/blob/master/include/coll/static_vector.h
  std::array<storage_t, len> buffer_{};
  typename std::array<storage_t, len>::iterator pos_{ std::begin(buffer_) };
};
}  // namespace detail

template <std::size_t circular_buffer_len = 1024>
class positioner {};

}  // namespace tfc::motor
