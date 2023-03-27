#pragma once

#include <functional>
#include <string_view>
#include <type_traits>

#include <glaze/core/common.hpp>

#include <tfc/confman/detail/config_rpc_client.hpp>
#include <tfc/confman/observable.hpp>

namespace asio = boost::asio;

namespace tfc::confman {

/// \brief configuration storage which maintains and keeps a storage type up to date
/// \tparam storage_t equality comparable and default constructible type
template <typename storage_t>
class config {
public:
  /// \brief construct config and deliver it to config manager
  /// \param key identification of this config storage, requires to be unique
  config(asio::io_context& ctx, std::string_view key) : key_(key), client_(ctx, key) {
    client_.alive(glz::write_json_schema<storage_t>(), std::bind(&config::on_alive, this, std::placeholders::_1));
  }

  /// \brief construct config and deliver it to config manager
  /// \param key identification of this config storage, requires to be unique
  /// \param def default values of given storage type
  config(asio::io_context& ctx, std::string_view key, storage_t&& def)
      : key_(key), storage_(std::forward<decltype(def)>(def)), client_(ctx, key) {}

  /// \brief get const access to storage
  /// \note can be used to assign observer to observable even though it is const
  [[nodiscard]] auto get() const noexcept -> storage_t const& { return storage_; }

  /// \brief override current config with the given value
  /// \param storage to be overridden with
  void set(storage_t&& storage) { storage_ = std::forward<decltype(storage)>(storage); }

private:

  void on_alive(std::expected<detail::alive_result, glz::rpc::error> const& ) {

  }

  std::string key_{};
  storage_t storage_{};
  detail::config_rpc_client client_;
};

}  // namespace tfc::confman
