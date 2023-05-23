#pragma once
#include <filesystem>
#include <system_error>

#include <boost/asio/io_context.hpp>

#include <tfc/confman/detail/change.hpp>

namespace tfc::stubs::confman {

namespace asio = boost::asio;

template <typename storage_t>
class file_storage {
public:
  using type = storage_t;

  file_storage(asio::io_context& ctx, std::filesystem::path const& file_path)
      : file_storage{ ctx, file_path, storage_t{} } {}

  file_storage(asio::io_context&, std::filesystem::path const&, auto&& default_value)
      : storage_{ std::forward<decltype(default_value)>(default_value) } {}

  auto error() const noexcept -> std::error_code const& {
    static std::error_code unused{};
    return unused;
  }

  auto file() const noexcept -> std::filesystem::path const& {
    static std::filesystem::path unused{};
    return unused;
  }

  auto value() const noexcept -> storage_t const& { return storage_; }
  auto access() noexcept -> storage_t& { return storage_; }

  auto operator->() const noexcept -> storage_t const* { return std::addressof(value()); }

  auto on_change(std::invocable auto&&) -> void { }

  using change = tfc::confman::detail::change<file_storage>;

  auto make_change() -> change { return change{ *this }; }

  auto set_changed() const noexcept -> std::error_code {
    std::invoke(set_changed_cb);
    return {};
  }

  storage_t storage_{};
  std::function<void(void)> set_changed_cb{ []() {} };
};

}
