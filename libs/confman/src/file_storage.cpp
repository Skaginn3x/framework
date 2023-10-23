#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <map>
#include <set>
#include <system_error>

#include <stduuid/uuid.h>

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
static auto get_minimum_retention_days() -> std::chrono::days {
  return std::chrono::days(get_environment_variable("TFC_CONFMAN_MIN_RETENTION_DAYS", 30));
}

/// \brief Fetches the value of the environment variable "TFC_CONFMAN_MIN_RETENTION_COUNT", default value is 4.
/// \return minimum retention count set in the environment variable
/// \example get_minimum_retention_count();
/// If the environment variable "TFC_CONFMAN_MIN_RETENTION_COUNT" is set to "6", the function returns 6.
/// Otherwise, the function returns 4.
static auto get_minimum_retention_count() -> int {
  return get_environment_variable("TFC_CONFMAN_MIN_RETENTION_COUNT", 4);
}

/// \brief Delete old files from the specified directory based on retention count and days. If the number
/// of files exceed the retention count AND if the file's last modification date surpasses the retention time,
/// \param directory path to directory containing files to possibly delete.
static auto delete_old_files(std::filesystem::path const& directory) -> void {
  int const retention_count = get_minimum_retention_count();
  std::chrono::days const retention_time = get_minimum_retention_days();

  // +2 for the file itself and the swap file
  int total_file_retention_count = retention_count + 2;

  if (int64_t const total_file_count =
          std::distance(std::filesystem::directory_iterator(directory), std::filesystem::directory_iterator{});
      total_file_count <= static_cast<int64_t>(total_file_retention_count)) {
    return;
  }

  std::map<std::filesystem::file_time_type, std::filesystem::path> file_times;

  for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directory)) {
    file_times.try_emplace(entry.last_write_time(), entry.path());
  }

  std::filesystem::file_time_type const current_time = std::filesystem::file_time_type::clock::now();

  uint64_t count = 0;
  for (auto it = file_times.begin();
       it != file_times.end() && count <= (file_times.size() - static_cast<uint64_t>(total_file_retention_count));
       ++it, ++count) {
    auto time_since_last_modified = current_time - it->first;
    if (time_since_last_modified > retention_time) {
      std::filesystem::remove(it->second);
    }
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

/// \brief Generate a unique filename using a UUID.
/// \param config_file file path used for generating a unique filename.
/// \return `std::filesystem::path` object containing the unique filename.
/// \example
/// auto original_path = std::filesystem::path("/etc/configs/settings.conf");
/// auto uuid_filename = generate_uuid_filename(original_path);
/// // Possible result: "/etc/configs/settings_d53c5117-ddfd-4b31-9e3d-acf9ea627fee.json"
static auto generate_uuid_filename(std::filesystem::path config_file) -> std::filesystem::path {
  return std::filesystem::path{ config_file.replace_extension().string() + "_" + get_uuid() + ".json" };
}

/// \brief Write content to a backup file and delete older files if they surpass retention criteria.
/// \param file_content content to be written to the new file.
/// \param file_path path to file
/// \return `std::error_code` with result of write operation
auto write_and_delete_old_files(std::string_view file_content, std::filesystem::path const& file_path) -> std::error_code {
  auto write_error{ write_to_file(generate_uuid_filename(file_path), file_content) };

  if (write_error) {
    return write_error;
  }

  delete_old_files(file_path.parent_path());  // take into account minimum 30 days and minimum 4 files
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
