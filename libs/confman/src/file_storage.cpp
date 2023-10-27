#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <map>
#include <optional>

#include <stduuid/uuid.h>

#include <tfc/utils/pragmas.hpp>

// clang-format off
// dynamic and clang build would fail if this warning is not deactivated
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wdate-time)
#include <pcg_random.hpp>
#include <pcg_extras.hpp>
PRAGMA_CLANG_WARNING_POP
// clang-format on

#include <tfc/confman/detail/retention.hpp>
#include <tfc/confman/file_storage.hpp>

namespace {

constexpr std::chrono::days default_retention_days{ 30 };
constexpr size_t default_retention_count{ 4 };

/// \brief fetch value of "TFC_CONFMAN_MIN_RETENTION_DAYS", if value is not set a default of std::chrono::days(30) is
/// returned
/// \return `std::chrono::days` set in the environment variable
auto get_minimum_retention_days() -> std::chrono::days {
  std::optional<size_t> const env = tfc::confman::getenv<size_t>("TFC_CONFMAN_MIN_RETENTION_DAYS");
  if (!env.has_value()) {
    return default_retention_days;
  }
  return std::chrono::days(env.value());
}

/// \brief Fetches the value of the environment variable "TFC_CONFMAN_MIN_RETENTION_COUNT", default value is 4.
/// \return minimum retention count set in the environment variable
auto get_minimum_retention_count() -> size_t {
  std::optional<size_t> const env = tfc::confman::getenv<size_t>("TFC_CONFMAN_MIN_RETENTION_COUNT");
  if (!env.has_value()) {
    return default_retention_count;
  }
  return env.value();
}

/// \brief Generates a universally unique identifier (UUID), generated UUID is compliant with the RFC 4122.
/// \returns std::string of generated UUID.
/// \example auto id = get_uuid(); id = "d53c5117-ddfd-4b31-9e3d-acf9ea627fee"
auto get_uuid() -> std::string {
  pcg_extras::seed_seq_from<std::random_device> seed_source;
  pcg64 random_engine(seed_source);
  uuids::basic_uuid_random_generator<pcg64> random_generator{ random_engine };
  return uuids::to_string(random_generator());
}

/// \brief Generate a unique filename using a UUID.
/// \param config_file file path used for generating a unique filename.
/// \return `std::filesystem::path` object containing the unique filename.
/// \example
/// auto original_path = std::filesystem::path("/etc/configs/settings.conf");
/// auto uuid_filename = generate_uuid_filename(original_path);
/// // Possible result: "/etc/configs/settings_d53c5117-ddfd-4b31-9e3d-acf9ea627fee.json"
auto generate_uuid_filename(std::filesystem::path config_file) -> std::filesystem::path {
  return std::filesystem::path{ config_file.replace_extension().string() + "_" + get_uuid() + ".json" };
}

/// \brief Delete old files from a specified configuration setting based on retention count and days. If the number
/// of files exceed the retention count AND if the file's last modification date surpasses the retention time,
/// \param file_path configuration to apply retention to
auto apply_retention_policy(std::filesystem::path const& file_path) -> void {
  // +1 for the file itself
  size_t const retention_count = get_minimum_retention_count() + 1;
  std::chrono::days const retention_time = get_minimum_retention_days();

  std::map<std::filesystem::file_time_type, std::filesystem::path> const file_times =
      tfc::confman::get_json_files_by_last_write_time(file_path);

  /// no need to apply retention if files are under the retention count
  if (file_times.size() <= retention_count) {
    return;
  }

  tfc::confman::remove_json_files_exceeding_retention(file_times, retention_count, retention_time);
}

}  // namespace

namespace tfc::confman {

/// \brief Write content to a backup file and delete older files if they surpass retention criteria.
/// \param file_content content to be written to the new file.
/// \param file_path path to file
/// \return `std::error_code` with result of write operation
auto write_and_apply_retention_policy(std::string_view file_content, std::filesystem::path const& file_path)
    -> std::error_code {
  auto write_error{ write_to_file(generate_uuid_filename(file_path), file_content) };

  if (write_error) {
    return write_error;
  }

  apply_retention_policy(file_path);
  return {};
}

/// \brief writes file contents to a file
/// \param file_path path of the file
/// \param file_contents contents of the file
/// \returns A std::error_code indicating success or failure. If write fails, IO error is returned.
auto write_to_file(std::filesystem::path const& file_path, std::string_view file_contents) -> std::error_code {
  auto glz_err{ glz::buffer_to_file(file_contents, file_path) };
  if (glz_err != glz::error_code::none) {
    return std::make_error_code(std::errc::io_error);
  }
  return {};
}

}  // namespace tfc::confman
