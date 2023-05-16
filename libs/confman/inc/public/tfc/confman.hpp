#pragma once

#include <functional>
#include <string_view>
#include <type_traits>

#include <glaze/glaze.hpp>

#include <tfc/confman/detail/change.hpp>
#include <tfc/confman/detail/config_dbus_client.hpp>
#include <tfc/confman/file_storage.hpp>
#include <tfc/progbase.hpp>

namespace tfc::confman {

namespace asio = boost::asio;

/// \brief configuration storage which maintains and keeps a storage type up to date
/// \tparam config_storage_t equality comparable and default constructible type
template <typename config_storage_t>
class config {
public:
  using type = config_storage_t;
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
        client_{ ctx, key, std::bind_front(&config::string, this), std::bind_front(&config::schema, this),
                 std::bind_front(&config::from_string, this) },
        logger_(fmt::format("config.{}", key)) {
    storage_.on_change([]() {
      // todo this can lead too callback hell, set property calls dbus set prop and dbus set prop calls back
      //      client_.set(detail::config_property{ .value = string(), .schema = schema() });
    });
  }

  /// \brief get const access to storage
  /// \note can be used to assign observer to observable even though it is const
  [[nodiscard]] auto value() const noexcept -> storage_t const& { return storage_.value(); }
  /// \brief accessor to given storage
  auto operator->() const noexcept -> storage_t const* { return std::addressof(value()); }

  /// \return storage_t as json string
  [[nodiscard]] auto string() const -> std::string { return glz::write_json(storage_.value()); }
  /// \return storage_t json schema
  [[nodiscard]] auto schema() const -> std::string { return glz::write_json_schema<config_storage_t>(); }

  auto set_changed() const noexcept -> std::error_code {
    client_.set(detail::config_property{ .value = string(), .schema = schema() });
    return storage_.set_changed();
  }

  /// \brief get config key used to index the given object of type storage_t
  [[nodiscard]] auto file() const noexcept -> std::filesystem::path const& { return storage_.file(); }

  using change = detail::change<config>;

  auto make_change() noexcept -> change { return change{ *this }; }

  auto from_string(std::string_view value) -> std::error_code {
    // this will call N nr of callbacks
    // for each confman::observer type
    auto const error{ glz::read_json<storage_t>(storage_.make_change().value(), value) };
    if (error) {
      return std::make_error_code(std::errc::io_error);  // todo make glz to std::error_code
    }
    return {};
  }

private:
  friend struct detail::change<config>;

  // todo if this could be named `value` it would be neat
  // the change mechanism relies on this (the friend above)
  // todo const_cast is not nice, make different pattern
  [[nodiscard]] auto access() noexcept -> storage_t& { return const_cast<storage_t&>(storage_.value()); }

  file_storage<storage_t> storage_{};
  detail::config_dbus_client client_;
  tfc::logger::logger logger_;
};

}  // namespace tfc::confman
