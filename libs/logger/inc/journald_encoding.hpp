#pragma once

#include <string_view>
#include <vector>

namespace tfc::logger::journald {
// Encode the given key value pairs according to https://systemd.io/JOURNAL_NATIVE_PROTOCOL/
auto to_message(std::vector<std::pair<std::string_view, std::string_view>>& fields) -> std::vector<char> {
  std::string newline = std::string("\n");
  std::vector<char> ret_value;
  for (auto& [key, value] : fields) {
    uint64_t value_size = value.size();
    std::string value_string(reinterpret_cast<char*>(&value_size), sizeof(value_size));
    std::copy_n(key.begin(), key.size(), std::back_inserter(ret_value));
    std::copy_n(newline.begin(), 1, std::back_inserter(ret_value));
    std::copy_n(value_string.begin(), value_string.size(), std::back_inserter(ret_value));
    std::copy_n(value.begin(), value.size(), std::back_inserter(ret_value));
    std::copy_n(newline.begin(), 1, std::back_inserter(ret_value));
  }
  return ret_value;
}
}  // namespace tfc::logger::journald
