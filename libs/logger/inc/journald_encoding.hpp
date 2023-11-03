#pragma once

import std;

namespace tfc::logger::journald {
// Encode the given key value pairs according to https://systemd.io/JOURNAL_NATIVE_PROTOCOL/
inline auto to_message(std::vector<std::pair<std::string_view, std::string_view>>& fields) -> std::vector<char> {
  std::vector<char> ret_value;
  for (auto& [key, value] : fields) {
    std::uint64_t value_size = value.size();
    std::string value_string(reinterpret_cast<char*>(&value_size), sizeof(value_size));
    std::copy_n(key.begin(), key.size(), std::back_inserter(ret_value));
    ret_value.emplace_back('\n');
    std::copy_n(value_string.begin(), value_string.size(), std::back_inserter(ret_value));
    std::copy_n(value.begin(), value.size(), std::back_inserter(ret_value));
    ret_value.emplace_back('\n');
  }
  return ret_value;
}
}  // namespace tfc::logger::journald
