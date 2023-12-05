#pragma once

namespace tfc::motor {

namespace detail {
template <typename storage_t, std::size_t len>
struct circular_buffer {
  circular_buffer() { buffer_.reserve(len); }
  template <typename... Args>
  inline constexpr auto emplace_back(Args&&... args) noexcept(tune == vector_tune_e::unchecked ||
                                                              noexcept(detail::emplace_back(*this,
                                                                                            std::forward<Args>(args)...))) {
    if constexpr (tune == vector_tune_e::checked)
      return detail::emplace_back(*this, std::forward<Args>(args)...);
    else
      return detail::emplace_back_unchecked(*this, std::forward<Args>(args)...);
  }

  // todo use static_vector, https://github.com/arturbac/small_vectors/blob/master/include/coll/static_vector.h
  std::vector<storage_t> buffer_{};
  std::vector<storage_t>::iterator pos_{ std::begin(buffer_) };
};
}  // namespace detail

template <std::size_t circular_buffer_len = 1024>
class tacho {};

}  // namespace tfc::motor
