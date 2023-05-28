#pragma once
#include <filesystem>
#include <string_view>
#include <system_error>

#include <boost/asio/io_context.hpp>

#include <tfc/confman.hpp>

namespace tfc::stubs::confman {

namespace asio = boost::asio;

template <typename config_storage_t>
class config : public tfc::confman::config<config_storage_t> {
public:
  using type = config_storage_t;
  using storage_t = config_storage_t;

  config(asio::io_context& ctx, std::string_view key) : config{ ctx, key, config_storage_t{} } {}

  template <typename storage_type>
    requires std::same_as<storage_t, std::remove_cvref_t<storage_type>>
  config(asio::io_context& ctx, std::string_view, storage_type&& def)
      : tfc::confman::config<config_storage_t>{ ctx }, storage_{ std::forward<storage_type>(def) } {}

  [[nodiscard]] auto value() const noexcept -> storage_t const& { return storage_; }
  auto access() noexcept -> storage_t& { return storage_; }
  auto operator->() const noexcept -> storage_t const* { return std::addressof(value()); }

  [[nodiscard]] auto string() const -> std::string { return glz::write_json(storage_); }
  [[nodiscard]] auto schema() const -> std::string { return glz::write_json_schema<storage_t>(); }

  auto set_changed() const noexcept -> std::error_code { return {}; }

  using change = tfc::confman::detail::change<config>;

  auto make_change() noexcept -> change { return change{ *this }; }

  auto from_string(std::string_view value) -> std::error_code { return {}; }

  storage_t storage_{};
  std::function<void(void)> set_changed_cb{ []() {} };
};

}  // namespace tfc::stubs::confman