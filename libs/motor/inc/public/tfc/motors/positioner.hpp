#pragma once

namespace tfc::motor {

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
  constexpr auto front() noexcept -> storage_t& {
    return *front_;
  }
  constexpr auto front() const noexcept -> storage_t const& {
    return *front_;
  }

  // todo use static_vector, https://github.com/arturbac/small_vectors/blob/master/include/coll/static_vector.h
  std::array<storage_t, len> buffer_{};
  // front is invalid when there has no item been inserted yet, but should not matter much
  typename std::array<storage_t, len>::iterator front_{ std::begin(buffer_) };
  typename std::array<storage_t, len>::iterator insert_pos_{ std::begin(buffer_) };
};
}  // namespace detail

template <std::size_t circular_buffer_len = 1024>
class positioner {};

}  // namespace tfc::motor
