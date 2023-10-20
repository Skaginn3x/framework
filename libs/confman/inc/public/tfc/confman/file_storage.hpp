#pragma once
#include <sys/inotify.h>
#include <filesystem>
#include <iostream>

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <set>

#include <fmt/format.h>
#include <boost/asio/io_context.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <glaze/glaze.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <tfc/confman/detail/change.hpp>
#include <tfc/logger.hpp>

namespace tfc::confman {

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
  auto make_change() noexcept -> change { return change{ *this }; }

  /// \brief set_changed writes the current value to disc
  /// \return error_code if it was unable to write to disc.
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

protected:
  friend struct detail::change<file_storage>;

  // todo if this could be named `value` it would be neat
  // the change mechanism relies on this (the friend above)
  auto access() noexcept -> storage_t& { return storage_; }

  auto read_file() -> std::error_code {
    std::string buffer{};
    auto glz_err{ glz::read_file_json(storage_, config_file_.string(), buffer) };
    if (glz_err) {
      logger_.warn(R"(Error: "{}" reading from file: "{}")", glz::write_json(glz_err.ec), config_file_.string());
      return std::make_error_code(std::errc::io_error);
      // todo implicitly convert glaze error_code to std::error_code
    }

    old_file = current_file;
    current_file = buffer;

    return {};
  }

  auto remove_json_ending(const std::string& input) -> std::string {
    const std::string suffix = ".json";

    if (input.size() >= suffix.size() && input.compare(input.size() - suffix.size(), suffix.size(), suffix) == 0) {
      return input.substr(0, input.size() - suffix.size());
    }
    return input;
  }

  auto backup_old_file() -> std::error_code {
    boost::uuids::uuid const uuid{ boost::uuids::random_generator()() };

    std::string old_file_s = remove_json_ending(config_file_.string()) + "_" + std::to_string(hash_value(uuid)) + ".json";

    auto glz_err{ glz::buffer_to_file(old_file, old_file_s) };
    if (glz_err != glz::error_code::none) {
      logger_.warn(R"(Error: "{}" writing to file: "{}")", glz::write_json(glz_err), old_file_s);
      return std::make_error_code(std::errc::io_error);
    }
    return {};
  }

  auto get_environment_variable(std::string env_variable, int default_value) -> int {
    const char* value = std::getenv(env_variable.c_str());

    if (value != nullptr) {
      try {
        int const value_i = std::stoi(value);
        if (value_i >= 0) {
          std::cout << env_variable << value_i << std::endl;
          return value_i;
        }
        logger_.error("Warning: Negative value encountered for {}. Using default", env_variable);
      } catch (...) {
        logger_.error("Invalid format for {}. Using default", env_variable);
      }
    } else {
      logger_.info("{} is not set. Using default.", env_variable);
    }

    return default_value;
  }

  auto get_minimum_retention_days() -> std::chrono::days {
    return std::chrono::days(get_environment_variable("TFC_CONFMAN_MIN_RETENTION_DAYS", 30));
  }

  auto get_minimum_retention_count() -> int { return get_environment_variable("TFC_CONFMAN_MIN_RETENTION_COUNT", 4); }

  auto delete_old_files() -> void {
    std::chrono::days retention_time = get_minimum_retention_days();

    std::filesystem::file_time_type current_time = std::filesystem::file_time_type::clock::now();

    for (const auto& entry : std::filesystem::directory_iterator(config_file_.parent_path())) {
      std::filesystem::file_time_type ftime = std::filesystem::last_write_time(entry);
      auto time_since_last_modified = current_time - ftime;
      if (time_since_last_modified > retention_time) {
        std::filesystem::remove(entry.path().string());
      }
    }
  }

  auto keep_newest_files() -> void {
    int total_file_retention_count = get_minimum_retention_count() + 2;

    int total_file_count = std::distance(std::filesystem::directory_iterator(config_file_.parent_path()),
                                         std::filesystem::directory_iterator{});

    if (total_file_count <= total_file_retention_count) {
      return;
    }

    std::map<std::filesystem::file_time_type, std::filesystem::path> file_times;

    for (const auto& entry : std::filesystem::directory_iterator(config_file_.parent_path())) {
      file_times.insert({ entry.last_write_time(), entry.path() });
    }

    uint64_t count = 0;
    for (auto it = file_times.begin(); it != file_times.end() && count <= (file_times.size() - total_file_retention_count);
         ++it, ++count) {
      std::filesystem::remove(it->second);
    }
  }

  auto on_file_change(std::error_code const& err, std::size_t) -> void {
    if (err) {
      fmt::print(stderr, "File watch error: {}\n", err.message());
      return;
    }
    std::array<char, 1024> buf{};
    file_watcher_.read_some(asio::buffer(buf));  // discard the data since we only got one watcher per inotify fd

    logger_.trace("File change");
    read_file();

    if (cb_) {
      std::invoke(cb_);
    }

    backup_old_file();
    delete_old_files();
    keep_newest_files();

    file_watcher_.async_read_some(asio::null_buffers(), std::bind_front(&file_storage::on_file_change, this));
  }

  std::filesystem::path config_file_{};
  storage_t storage_{};
  tfc::logger::logger logger_;
  std::error_code error_{};
  asio::posix::stream_descriptor file_watcher_;
  std::function<void()> cb_{};
  std::string current_file;
  std::string old_file;
};

}  // namespace tfc::confman
