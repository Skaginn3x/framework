#pragma once

#include <functional>
#include <string_view>
#include <type_traits>

#include <glaze/core/common.hpp>

#include <tfc/confman/detail/change.hpp>
#include <tfc/confman/file_storage.hpp>
#include <tfc/confman/detail/config_dbus_client.hpp>
#include <tfc/progbase.hpp>

namespace tfc::confman {

namespace asio = boost::asio;

/// \brief configuration storage which maintains and keeps a storage type up to date
/// \tparam config_storage_t equality comparable and default constructible type
template <typename config_storage_t>
class config {
public:
  using storage_t = config_storage_t;

  /// \brief construct config and deliver it to config manager
  /// \param ctx context ref to which the config shall run in
  /// \param key identification of this config storage, requires to be unique
  config(asio::io_context& ctx, std::string_view key) : config{ ctx, key, config_storage_t{} } {}

  /// \brief construct config and deliver it to config manager
  /// \param ctx context ref to which the config shall run in
  /// \param key identification of this config storage, requires to be unique
  /// \param def default values of given storage type
  template <typename storage_type>
    requires std::same_as<storage_t, std::remove_cvref_t<storage_type>>
  config(asio::io_context& ctx, std::string_view key, storage_type&& def)
      : storage_{ ctx, tfc::base::make_config_file_name(key, "json"), std::forward<storage_type>(def) },
        client_{ ctx },
        logger_(fmt::format("config.{}", key)) {}

  /// \brief get const access to storage
  /// \note can be used to assign observer to observable even though it is const
  [[nodiscard]] auto value() const noexcept -> storage_t const& { return storage_.value(); }
  /// \brief accessor to given storage
  auto operator->() const noexcept -> storage_t const* { return std::addressof(value()); }

  auto set_changed() const noexcept -> std::error_code {
    return storage_.set_changed();
  }

  /// \brief get config key used to index the given object of type storage_t
  [[nodiscard]] auto file() const noexcept -> std::filesystem::path const& { return storage_.file(); }

  using change = detail::change<config>;
  friend struct detail::change<config>;

  auto make_change() noexcept -> change { return change{ *this }; }

private:
  file_storage<storage_t> storage_{};
  detail::config_dbus_client client_;
  tfc::logger::logger logger_;
};

}  // namespace tfc::confman
