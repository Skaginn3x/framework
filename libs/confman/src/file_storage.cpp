#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <system_error>

#include <stduuid/uuid.h>
#include <glaze/glaze.hpp>

#include <tfc/utils/pragmas.hpp>

// clang-format off
// dynamic and clang build would fail if this warning is not deactivated
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wdate-time)
#include <pcg_random.hpp>
#include <pcg_extras.hpp>
PRAGMA_CLANG_WARNING_POP
// clang-format on

#include <tfc/confman/file_storage.hpp>

namespace tfc::confman {

/// \brief Retains the newest files in a directory, deleting older ones.
/// This function iterates through the files in a given directory and if the directory contains too many files
/// it deletes the older ones.
/// \tparam parent_path directory path
/// \tparam retention_count number of files to retain
/// \example retain_newest_files("/path/to/directory", 5);
/// This ensures that only 5 backup files are retained in the "/path/to/directory", and older ones are deleted.
auto retain_newest_files(std::filesystem::path const& parent_path, int retention_count) -> void {
  // +2 for the file itself and the swap file
  int total_file_retention_count = retention_count + 2;

  if (int64_t total_file_count =
          std::distance(std::filesystem::directory_iterator(parent_path), std::filesystem::directory_iterator{});
      total_file_count <= static_cast<int64_t>(total_file_retention_count)) {
    return;
  }

  std::map<std::filesystem::file_time_type, std::filesystem::path> file_times;

  for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(parent_path)) {
    file_times.try_emplace(entry.last_write_time(), entry.path());
  }

  uint64_t count = 0;
  for (auto it = file_times.begin();
       it != file_times.end() && count <= (file_times.size() - static_cast<uint64_t>(total_file_retention_count));
       ++it, ++count) {
    std::filesystem::remove(it->second);
  }
}

/// \brief Generates a universally unique identifier (UUID), generated UUID is compliant with the RFC 4122.
/// \returns std::string of generated UUID.
/// \example auto id = get_uuid(); id = "d53c5117-ddfd-4b31-9e3d-acf9ea627fee"
static auto get_uuid() -> std::string {
  pcg_extras::seed_seq_from<std::random_device> seed_source;
  pcg64 random_engine(seed_source);
  uuids::basic_uuid_random_generator<pcg64> random_generator{ random_engine };
  return uuids::to_string(random_generator());
}

/// \brief Creates a backup of a file in the same directory as the file, appends a UUID to the file name.
/// \param file_path file to back up
/// \param file_contents contents of the file that should be backed up.
/// \returns A std::error_code indicating success or failure. If backup creation fails, IO error is returned.
/// \example backup_file("/etc/config", "configuration data");
/// A backup file named "/etc/config_<UUID>.json" is created with contents being "configuration data".
auto backup_file(std::filesystem::path& file_path, std::string_view file_contents) -> std::error_code {
  std::string const backup_file_name = file_path.replace_extension().string() + "_" + get_uuid() + ".json";

  auto glz_err{ glz::buffer_to_file(file_contents, backup_file_name) };
  if (glz_err != glz::error_code::none) {
    return std::make_error_code(std::errc::io_error);
  }
  return {};
}

/// \brief Deletes files older than a specified retention time.
/// \param parent_path directory containing files to check for deletion
/// \param retention_time duration of time (in days) for which files should be retained. Older files will be deleted.
/// \example delete_old_files("/etc/logs", std::chrono::days(30));
/// All files inside the "/etc/logs" directory that were last modified more than 30 days ago will be deleted
auto delete_old_files(std::filesystem::path const& parent_path, std::chrono::days const& retention_time) -> void {
  std::filesystem::file_time_type const current_time = std::filesystem::file_time_type::clock::now();

  for (const auto& entry : std::filesystem::directory_iterator(parent_path)) {
    auto time_since_last_modified = current_time - std::filesystem::last_write_time(entry);
    if (time_since_last_modified > retention_time) {
      std::filesystem::remove(entry.path().string());
    }
  }
}

/// \brief Retrieves an integer value of an environment variable, if a non-negative value is present on the variable
/// it is returned. Otherwise a default value is returned.
/// \param env_variable environment variable to fetch
/// \param default_value default integer value to be returned if the environment variable is not set or holds an invalid
/// value. \return value of the environment variable or the default value. \example
/// get_environment_variable("MAX_CONNECTIONS", 5); If the environment variable "MAX_CONNECTIONS" is set to "10", the
/// function returns 10. Otherwise, the function returns 5.
static auto get_environment_variable(std::string const& env_variable, int default_value) -> int {
  if (const char* value = std::getenv(env_variable.c_str()); value != nullptr) {
    if (int const value_i = std::stoi(value); value_i >= 0) {
      return value_i;
    }
  }
  return default_value;
}

/// \brief Fetches the value of the environment variable "TFC_CONFMAN_MIN_RETENTION_DAYS", default value is 30 days.
/// \return `std::chrono::days` set in the environment variable
/// \example get_minimum_retention_days();
/// If the environment variable "TFC_CONFMAN_MIN_RETENTION_DAYS" is set to "15", the function returns std::chrono::days(15).
/// Otherwise, the function returns std::chrono::days(30).
auto get_minimum_retention_days() -> std::chrono::days {
  return std::chrono::days(get_environment_variable("TFC_CONFMAN_MIN_RETENTION_DAYS", 30));
}

/// \brief Fetches the value of the environment variable "TFC_CONFMAN_MIN_RETENTION_COUNT", default value is 4.
/// \return minimum retention count set in the environment variable
/// \example get_minimum_retention_count();
/// If the environment variable "TFC_CONFMAN_MIN_RETENTION_COUNT" is set to "6", the function returns 6.
/// Otherwise, the function returns 4.
auto get_minimum_retention_count() -> int {
  return get_environment_variable("TFC_CONFMAN_MIN_RETENTION_COUNT", 4);
}

}  // namespace tfc::confman
