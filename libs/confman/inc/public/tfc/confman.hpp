#pragma once

#include <functional>
#include <string_view>
#include <type_traits>

#include <glaze/core/common.hpp>

#include <tfc/confman/detail/config_rpc_client_impl.hpp>
#include <tfc/confman/observable.hpp>

namespace tfc::confman {

namespace detail {
class config_rpc_client_pimpl {
public:
  config_rpc_client_pimpl() = default;
  void set_defaults(std::string_view key, std::string_view defaults);
  void override(std::string_view key, std::string_view value);
};
}  // namespace detail

/// \brief configuration storage which maintains and keeps a storage type up to date
/// \tparam storage_t equality comparable and default constructible type
template <typename storage_t>
class config {
public:
  /// \brief construct config and deliver it to config manager
  /// \param key identification of this config storage, requires to be unique
  explicit config(std::string_view key) : key_(key) {}

  /// \brief construct config and deliver it to config manager
  /// \param key identification of this config storage, requires to be unique
  /// \param def default values of given storage type
  config(std::string_view key, storage_t&& def) : key_(key), storage_(std::forward<decltype(def)>(def)) {}

  /// \brief get const access to storage
  /// \note can be used to assign observer to observable
  [[nodiscard]] auto get() const noexcept -> storage_t const& { return storage_; }

  /// \brief override current config with the given value
  /// \param storage to be overridden with
  void set(storage_t&& storage) { storage_ = std::forward<decltype(storage)>(storage); }

private:
  std::string key_{};
  storage_t storage_{};
};

}  // namespace tfc::confman
