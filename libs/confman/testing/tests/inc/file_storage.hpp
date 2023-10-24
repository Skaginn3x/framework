#pragma once

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <map>

namespace tfc::confman {

void remove_files_exceeding_retention(const std::map<std::filesystem::file_time_type, std::filesystem::path>& file_times,
                                      uint64_t retention_count,
                                      std::chrono::days retention_time);

auto get_minimum_retention_count() -> size_t;

auto get_minimum_retention_days() -> std::chrono::days;

}  // namespace tfc::confman
