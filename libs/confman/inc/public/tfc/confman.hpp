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
  /// \param ctx context to which the config shall run in
  /// \param key identification of this config storage, requires to be unique
  /// \param alive_cb callback called after the storage has been populated
  ///                 the input parameter of the callback is reference to `this` (self)
  config(asio::io_context& ctx, std::string_view key, std::invocable<config const&> auto&& alive_cb) : client_{ ctx, key } {
    init(std::forward<decltype(alive_cb)>(alive_cb));
  }

  /// \brief construct config and deliver it to config manager
  /// \param ctx context to which the config shall run in
  /// \param key identification of this config storage, requires to be unique
  /// \param alive_cb callback called after the storage has been populated
  ///                 the input parameter of the callback is reference to `this` (self)
  /// \param def default values of given storage type
  config(asio::io_context& ctx,
         std::string_view key,
         std::invocable<config const&> auto&& alive_cb,
         std::same_as<storage_t> auto&& def)
      : storage_(std::forward<decltype(def)>(def)), client_(ctx, key) {
    init(std::forward<decltype(alive_cb)>(alive_cb));
  }

  /// \brief get const access to storage
  /// \note can be used to assign observer to observable even though it is const
  [[nodiscard]] auto get() const noexcept -> storage_t const& { return storage_; }

  /// \brief override current config with the given value
  /// \param storage to be overridden with
  void set(std::same_as<storage_t> auto&& storage) { storage_ = std::forward<decltype(storage)>(storage); }

  auto key() const noexcept -> std::string_view { return client_.topic(); }

private:
  void init(std::invocable<config const&> auto&& alive_cb) {
    client_.alive(glz::write_json_schema<storage_t>(), glz::write_json(storage_),
                  [this, callback = alive_cb](auto const& val) {
                    on_alive(val);
                    std::invoke(callback, *this);
                  });
    client_.subscribe([this](std::expected<std::string_view, std::error_code> const& res) {
      if (res) {
        auto storage{ glz::read_json<storage_t>(res.value()) };
        if (storage) {
          set(std::move(storage.value()));
          return;
        }
      }
      // todo log error
    });
  }

  void on_alive(std::expected<detail::method::alive_result, glz::rpc::error> const& res) {
    if (res) {
      auto storage{ glz::read_json<storage_t>(res.value().config.str) };
      if (storage) {
        set(std::move(storage.value()));
        return;
      }
    }
    // Todo log error
  }

  storage_t storage_{};
  detail::config_rpc_client client_;
};

}  // namespace tfc::confman
