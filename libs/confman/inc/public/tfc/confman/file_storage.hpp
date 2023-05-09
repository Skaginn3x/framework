#pragma once
#include <sys/inotify.h>
#include <filesystem>

#include <fmt/format.h>
#include <boost/asio/io_context.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <glaze/glaze.hpp>

#include <tfc/confman/detail/change.hpp>
#include <tfc/logger.hpp>

namespace tfc::confman {

namespace asio = boost::asio;

/// \tparam storage_t needs to be transposable via glaze
/// \class file_storage
/// Blocking file storage, when
template <typename storage_t>
class file_storage {
public:
  file_storage(asio::io_context& ctx, std::filesystem::path file_path) : file_storage{ ctx, file_path, storage_t{} } {}

  file_storage(asio::io_context& ctx, std::filesystem::path file_path, auto&& default_value)
      : config_file_{ file_path }, storage_{ std::forward<decltype(default_value)>(default_value) },
        logger_{ fmt::format("file_storage.{}", file_path.string()) }, file_watcher_{ ctx } {
    std::filesystem::create_directories(config_file_.parent_path());
    error_ = read_file();
    if (error_) {
      error_ = set_changed();  // write to file
      if (error_) {
        return;
      }
    }
    auto const inotify_fd{ inotify_init1(IN_NONBLOCK) };
    if (inotify_fd < 0) {
      int const err{ errno };
      error_ = std::make_error_code(static_cast<std::errc>(err));
      return;
    }
    auto const inotify_watch_fd{ inotify_add_watch(inotify_fd, config_file_.c_str(), IN_MODIFY) };
    if (inotify_watch_fd < 0) {
      int const err{ errno };
      error_ = std::make_error_code(static_cast<std::errc>(err));
      return;
    }
    file_watcher_.assign(inotify_fd);
    file_watcher_.async_read_some(asio::null_buffers(), std::bind_front(&file_storage::on_file_change, this));
  }

  auto error() const noexcept -> std::error_code const& { return error_; }

  auto file() const noexcept -> std::filesystem::path const& { return config_file_; }

  /// \return Access to underlying value
  auto value() const noexcept -> storage_t const& { return storage_; }

  /// \return Access to underlying value
  auto operator->() const noexcept -> storage_t* { return std::addressof(value()); }

  auto on_change(std::invocable auto&& callback) -> void { cb_ = callback; }

  using change = detail::change<file_storage>;

  auto make_change() -> change { return change{ *this }; }

  // private: // todo why cannot value be made private here
  friend struct detail::change<file_storage>;
  auto value() noexcept -> storage_t& { return storage_; }
  auto operator->() noexcept -> storage_t* { return std::addressof(value()); }
  auto set_changed() const noexcept -> std::error_code {
    std::string buffer{};  // this can throw, meaning memory error
    glz::write<glz::opts{ .prettify = true }>(storage_, buffer);
    auto glz_err{ glz::buffer_to_file(buffer, config_file_.string()) };
    if (glz_err != glz::error_code::none) {
      logger_.warn(R"(Error: "{}" writing to file: "{}")", glz::write_json(glz_err), config_file_.string());
      return std::make_error_code(std::errc::io_error);
      // todo implicitly convert glaze error_code to std::error_code
    }
    return {};
  }

private:
  auto read_file() -> std::error_code {
    auto glz_err{ glz::read_file_json(storage_, config_file_.string()) };
    if (glz_err) {
      logger_.warn(R"(Error: "{}" reading from file: "{}")", glz::write_json(glz_err.ec), config_file_.string());
      return std::make_error_code(std::errc::io_error);
      // todo implicitly convert glaze error_code to std::error_code
    }
    return {};
  }

  auto on_file_change(std::error_code const& err, std::size_t) -> void {
    if (err) {
      logger_.warn("File watch error: {}", err.message());
      return;
    }
    std::array<char, 1024> buf{};
    file_watcher_.read_some(asio::buffer(buf));  // discard the data since we only got one watcher per inotify fd

    logger_.trace("File change");
    read_file();

    if (cb_) {
      std::invoke(cb_);
    }

    file_watcher_.async_read_some(asio::null_buffers(), std::bind_front(&file_storage::on_file_change, this));
  }

  std::filesystem::path config_file_{};
  storage_t storage_{};
  tfc::logger::logger logger_;
  std::error_code error_{};
  asio::posix::stream_descriptor file_watcher_;
  std::function<void()> cb_{};
};

}  // namespace tfc::confman
