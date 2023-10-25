#pragma once

#include <sys/inotify.h>
#include <filesystem>
#include <string>

#include <fmt/format.h>
#include <boost/asio/io_context.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <glaze/glaze.hpp>

#include <tfc/confman/detail/change.hpp>
#include <tfc/logger.hpp>

namespace tfc::confman {

auto write_to_file(const std::filesystem::path& file_path, std::string_view file_contents) -> std::error_code;
auto write_and_apply_retention_policy(std::string_view file_content, std::filesystem::path const& file_path)
    -> std::error_code;
// auto generate_uuid_filename(std::filesystem::path config_file) -> std::filesystem::path;
// auto apply_retention_policy(std::filesystem::path const& directory) -> void;

namespace asio = boost::asio;

/// \tparam storage_t needs to be transposable via glaze https://github.com/stephenberry/glaze
/// \class file_storage
/// The file storage class stores any type that can be converted to and from json.
/// The type is stored on the disc given the file_path as pretty json string.
/// If the file is changed while program is running the application detects the change and
/// changes the member value accordingly.
template <typename storage_t>
class file_storage {
public:
  using type = storage_t;

  /// \brief Empty constructor
  /// \note Should only be used for testing !!!
  explicit file_storage(asio::io_context& ctx) : logger_{ "file_storage" }, file_watcher_{ ctx } {}

  /// \brief Construct file storage with default constructed storage_t
  file_storage(asio::io_context& ctx, std::filesystem::path const& file_path)
      : file_storage{ ctx, file_path, storage_t{} } {}

  /// \brief Construct file storage with user defined default values for storage_t
  file_storage(asio::io_context& ctx, std::filesystem::path const& file_path, auto&& default_value)
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

  /// \brief Internal error code
  /// \returns error if something went wrong with filesystem commands
  [[nodiscard]] auto error() const noexcept -> std::error_code const& { return error_; }

  /// \return provided filesystem path
  [[nodiscard]] auto file() const noexcept -> std::filesystem::path const& { return config_file_; }

  /// \return Access to underlying value
  [[nodiscard]] auto value() const noexcept -> storage_t const& { return storage_; }

  /// \return Access to underlying value
  auto operator->() const noexcept -> storage_t const* { return std::addressof(value()); }

  /// \brief Subscribe to changes from the filesystem
  auto on_change(std::invocable auto&& callback) -> void { cb_ = callback; }

  using change = detail::change<file_storage>;

  /// If user would like to change the internal storage_t value.
  /// This helper struct will provide changeable access to this` underlying value.
  /// When the helper struct is deconstructed the changes are written to the disc.
  /// \return change helper struct providing reference to this` value.
  auto make_change() noexcept -> change {
    auto write_error{ write_and_apply_retention_policy(to_json(), config_file_) };

    if (write_error) {
      logger_.error("Error writing to file: ", write_error.message());
    }

    return change{ *this };
  }

  /// \brief set_changed writes the current value to disc
  /// \return error_code if it was unable to write to disc.
  auto set_changed() const noexcept -> std::error_code {
    if (auto write_error{ write_to_file(config_file_, to_json()) }; write_error) {
      logger_.warn(R"(Error: "{}" writing to file: "{}")", write_error.message(), config_file_.string());
      return write_error;
    }
    return {};
  }

  /// \brief generate json form of storage
  auto to_json() const noexcept -> std::string {
    std::string buffer{};  // this can throw, meaning memory error
    glz::write<glz::opts{ .prettify = true }>(storage_, buffer);
    return buffer;
  }

protected:
  friend struct detail::change<file_storage>;

  // todo if this could be named `value` it would be neat
  // the change mechanism relies on this (the friend above)
  auto access() noexcept -> storage_t& { return storage_; }

  auto read_file() -> std::error_code {
    std::string buffer{};
    if (auto glz_err{ glz::read_file_json(storage_, config_file_.string(), buffer) }; glz_err) {
      logger_.warn(R"(Error: "{}" reading from file: "{}")", glz::write_json(glz_err.ec), config_file_.string());
      return std::make_error_code(std::errc::io_error);
      // todo implicitly convert glaze error_code to std::error_code
    }
    return {};
  }

  auto on_file_change(std::error_code const& err, std::size_t) -> void {
    if (err) {
      fmt::print(stderr, "File watch error: {}\n", err.message());
      return;
    }
    std::array<char, 1024> buf{};
    file_watcher_.read_some(asio::buffer(buf));  // discard the data since we only got one watcher per inotify fd

    logger_.trace("File change");

    auto write_error{ write_and_apply_retention_policy(to_json(), config_file_) };

    if (write_error) {
      logger_.error("Error writing to file: ", write_error.message());
    }

    // the following updates internal storage_ member
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
