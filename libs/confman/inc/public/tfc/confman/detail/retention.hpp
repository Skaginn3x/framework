#pragma once

#include <chrono>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>

namespace tfc::confman {

static constexpr std::string_view json_file_ending{ ".json" };

/// \brief get map of specified configuration in directory, sorting them by time
/// \param file_path configuration file to get backups from
static auto get_json_files_by_last_write_time(std::filesystem::path const& file_path)
    -> std::map<std::filesystem::file_time_type, std::filesystem::path> {
  std::map<std::filesystem::file_time_type, std::filesystem::path> file_times;

  auto make_filter = [file_path](std::filesystem::directory_entry entry) noexcept {
    /// get all files that have .json ending and have same start letters
    /// example: test_file.json, should return true on test_file_1.json and test_file_2.json, but not test.json
    const auto& stem = file_path.stem().string();
    return entry.path().extension() == json_file_ending && entry.path().stem().string().substr(0, stem.size()) == stem;
  };

  for (const std::filesystem::directory_entry& entry :
       std::filesystem::directory_iterator(file_path.parent_path()) | std::views::filter(make_filter)) {
    file_times.try_emplace(entry.last_write_time(), entry.path());
  }
  return file_times;
}

/// \brief removes files that exceed the specified retention policy
/// \param file_times map containing json files sorted by their modification time
/// \param retention_count maximum number of files to retain
/// \param retention_time maximum age of files to retain
/// \note in order for a file to be removed, it both needs to be too old and too many files
static auto remove_json_files_exceeding_retention(
    const std::map<std::filesystem::file_time_type, std::filesystem::path>& file_times,
    size_t retention_count,
    std::chrono::days retention_time) -> void {
  auto const current_time = std::filesystem::file_time_type::clock::now();

  size_t count = 0;
  for (auto const& [time, path] : file_times) {
    count++;
    if (count > retention_count && (current_time - time) > retention_time) {
      std::filesystem::remove(path);
    }
  }
}

template <typename type_t>
  requires(std::integral<type_t> || std::same_as<type_t, std::string>)
auto getenv(std::string_view name) -> std::optional<type_t> {
  decltype(auto) value_str = std::getenv(std::string{ name }.c_str());
  if (value_str == nullptr) {
    return std::nullopt;
  }
  std::optional<type_t> return_val;
  try {
    if constexpr (std::same_as<type_t, std::uint64_t>) {
      return_val = std::stoul(value_str);
    } else if constexpr (std::integral<type_t>) {
      return_val = std::stoi(value_str);
    } else {
      return_val = value_str;
    }
  } catch (const std::invalid_argument&) {
    return std::nullopt;
  } catch (const std::out_of_range&) {
    return std::nullopt;
  }
  return return_val;
}

}  // namespace tfc::confman
