#pragma once

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <map>

namespace tfc::confman {

/// \brief removes files that exceed the specified retention policy
/// \param file_times map containing files sorted by their modification time
/// \param retention_count maximum number of files to retain
/// \param retention_time maximum age of files to retain
/// \note in order for a file to be removed, it both needs to be too old and too many files
static auto remove_files_exceeding_retention(
    const std::map<std::filesystem::file_time_type, std::filesystem::path>& file_times,
    size_t retention_count,
    std::chrono::days retention_time) -> void {
  std::filesystem::file_time_type const current_time = std::filesystem::file_time_type::clock::now();

  size_t count = 0;
  for (auto const& [time, path] : file_times) {
    count++;
    if (count > retention_count && (current_time - time) > retention_time) {
      std::filesystem::remove(path);
    }
  }
}

}  // namespace tfc::confman
