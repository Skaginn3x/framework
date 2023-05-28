#pragma once
#include <filesystem>
#include <system_error>

#include <boost/asio/io_context.hpp>

#include <tfc/confman/file_storage.hpp>

namespace tfc::confman {

namespace asio = boost::asio;

/// \brief file_storage stub class removing every file system operations
template <typename storage_t>
class stub_file_storage : public file_storage<storage_t> {
public:
  using type = storage_t;

  stub_file_storage(asio::io_context& ctx, std::filesystem::path const& file_path)
      : stub_file_storage{ ctx, file_path, storage_t{} } {}

  stub_file_storage(asio::io_context& ctx, std::filesystem::path const&, auto&& default_value)
      : file_storage<storage_t>{ ctx } {
    this->storage_ = std::forward<decltype(default_value)>(default_value);
  }

  auto value() const noexcept -> storage_t const& { return this->storage_; }
  auto access() noexcept -> storage_t& { return this->storage_; }

  auto operator->() const noexcept -> storage_t const* { return std::addressof(value()); }

  using change = tfc::confman::detail::change<stub_file_storage>;

  auto make_change() -> change { return change{ *this }; }

  auto set_changed() const noexcept -> std::error_code {
    std::invoke(this->cb_);
    return {};
  }
};

}  // namespace tfc::stubs::confman
