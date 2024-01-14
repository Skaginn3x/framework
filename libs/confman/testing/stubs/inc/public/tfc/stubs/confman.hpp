#pragma once
#include <filesystem>
#include <string_view>
#include <system_error>

#include <boost/asio/io_context.hpp>

#include <tfc/confman.hpp>
#include <tfc/stubs/confman/detail/config_dbus_client.hpp>
#include <tfc/stubs/confman/file_storage.hpp>

namespace tfc::confman {

namespace asio = boost::asio;

namespace detail {
template <typename config_storage_t>
using stubbed_config = config<config_storage_t, stub_file_storage<config_storage_t>, stub_config_dbus_client>;
}

template <typename config_storage_t, typename = void, typename = void>
class stub_config : public detail::stubbed_config<config_storage_t> {
public:
  using type = config_storage_t;
  using storage_t = config_storage_t;

  stub_config(asio::io_context& ctx, std::string_view key) : stub_config{ ctx, key, config_storage_t{} } {}

  template <typename storage_type>
    requires std::same_as<storage_t, std::remove_cvref_t<storage_type>>
  stub_config(asio::io_context& ctx, std::string_view key, storage_type&& def)
      : detail::stubbed_config<config_storage_t>{ ctx, key, std::forward<storage_type>(def) },
        storage_{ std::forward<storage_type>(def) } {}

  stub_config(std::shared_ptr<sdbusplus::asio::connection> conn, std::string_view key)
      : stub_config{ conn, key, config_storage_t{} } {}

  template <typename storage_type>
    requires std::same_as<storage_t, std::remove_cvref_t<storage_type>>
  stub_config(std::shared_ptr<sdbusplus::asio::connection> conn, std::string_view key, storage_type&& def)
  : stub_config{ conn->get_io_context(), key, std::forward<storage_type>(def) } {}

  [[nodiscard]] auto value() const noexcept -> storage_t const& { return storage_; }
  auto access() noexcept -> storage_t& { return storage_; }
  auto operator->() const noexcept -> storage_t const* { return std::addressof(value()); }

  [[nodiscard]] auto string() const -> std::string { return glz::write_json(storage_); }
  [[nodiscard]] auto schema() const -> std::string { return glz::write_json_schema<storage_t>(); }

  auto set_changed() const noexcept -> std::error_code { return {}; }

  using change = detail::change<stub_config>;

  auto make_change() noexcept -> change { return change{ *this }; }

  auto from_string(std::string_view value) -> std::error_code {
    auto const error{ glz::read_json<storage_t>(make_change().value(), value) };
    if (error) {
      return std::make_error_code(std::errc::io_error);  // todo make glz to std::error_code
    }
    return {};
  }

  storage_t storage_{};
};

}  // namespace tfc::confman
