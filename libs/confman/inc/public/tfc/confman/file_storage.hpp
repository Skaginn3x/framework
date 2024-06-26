#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>

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
  explicit file_storage(asio::io_context&) : logger_{ "file_storage" } {}

  /// \brief Construct file storage with default constructed storage_t
  file_storage(asio::io_context& ctx, std::filesystem::path const& file_path)
      : file_storage{ ctx, file_path, storage_t{} } {}

  /// \brief Construct file storage with user defined default values for storage_t
  file_storage(asio::io_context&, std::filesystem::path const& file_path, auto&& default_value)
      : config_file_{ file_path }, storage_{ std::forward<decltype(default_value)>(default_value) },
        logger_{ fmt::format("file_storage.{}", file_path.string()) } {
    std::filesystem::create_directories(config_file_.parent_path());
    error_ = read_file();
    if (error_) {
      // The file does not exist
      if (!std::filesystem::exists(config_file_) || std::filesystem::file_size(config_file_) == 0) {
        error_ = set_changed();
        if (error_) {
          throw std::runtime_error(fmt::format("Unable to write configuration file to disc {}", config_file_.string()));
        }
      } else {
        // When unable to parse configuration throw a runtime error. This is a fatal error.
        std::string message =
            fmt::format("Unable to read config file: {}, err: {}, throwing!", config_file_.string(), error_.message());
        logger_.error(message);
        throw std::runtime_error(message);
      }
    }
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

  using change = detail::change<file_storage>;

  /// If user would like to change the internal storage_t value.
  /// This helper struct will provide changeable access to this` underlying value.
  /// When the helper struct is deconstructed the changes are written to the disc.
  /// \return change helper struct providing reference to this` value.
  auto make_change() noexcept -> change {
    if (auto write_error{ write_and_apply_retention_policy(to_json(), config_file_) }; write_error) {
      logger_.warn(R"(Error: "{}" writing to file: "{}")", write_error.message(), config_file_.string());
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
    auto const err{ glz::write<glz::opts{ .prettify = true }>(storage_, buffer) };
    if (err) {
      logger_.error(R"(Error: "{}" writing to json)", glz::format_error(err));
      return {};
    }
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
      logger_.warn(R"(Error: "{}" reading from file: "{}")", glz::format_error(glz_err, buffer), config_file_.string());
      return std::make_error_code(std::errc::io_error);
      // todo implicitly convert glaze error_code to std::error_code
    }
    return {};
  }

  std::filesystem::path config_file_{};
  storage_t storage_{};
  tfc::logger::logger logger_;
  std::error_code error_{};
};

}  // namespace tfc::confman
