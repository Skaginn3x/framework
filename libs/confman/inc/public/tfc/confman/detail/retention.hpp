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

/// \brief removes files that exceed the specified retention policy
/// \param file_times map containing files sorted by their modification time
/// \param retention_count maximum number of files to retain
/// \param retention_time maximum age of files to retain
/// \note in order for a file to be removed, it both needs to be too old and too many files
static auto remove_files_exceeding_retention(
    const std::map<std::filesystem::file_time_type, std::filesystem::path>& file_times,
    size_t retention_count,
    std::chrono::days retention_time) -> void {
  auto const current_time = std::filesystem::file_time_type::clock::now();

  constexpr auto make_json_filter = [](const auto& pair) noexcept { return pair.second.extension() == json_file_ending; };

  size_t count = 0;
  for (auto const& [time, path] : file_times | std::views::filter(make_json_filter)) {
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
