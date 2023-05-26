#pragma once

#include <memory>

namespace tfc::confman::detail {

template <typename owner_t>
/// \struct
/// Make changes for the owning object. Changes will be executed during deconstruction.
/// \note should never be used as long living object.
///       as it owns reference to owner.
/// \tparam owner_t Owning object type.
struct change {
  explicit change(owner_t& owner) noexcept : owner_{ owner } {}

  change(change const&) noexcept = default;
  change(change&&) noexcept = default;
  change& operator=(change const&) noexcept = default;
  change& operator=(change&&) noexcept = default;

  /// \brief let owner know of the changes
  ~change() {
    owner_.set_changed();  // todo: good or bad?, evidently need to add concept to typename owner_t
  }

  /// \return Access to underlying owner value
  [[nodiscard]] auto value() noexcept -> auto& { return owner_.access(); }

  /// \return Access to underlying owner value
  auto operator->() noexcept -> auto* { return std::addressof(owner_.access()); }

private:
  owner_t& owner_;
};

}  // namespace tfc::confman::detail
