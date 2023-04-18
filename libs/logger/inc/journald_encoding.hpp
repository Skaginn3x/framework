#pragma once

#include <string_view>
#include <vector>

namespace tfc::logger::journald {
  // Encode the given key value pairs according to https://systemd.io/JOURNAL_NATIVE_PROTOCOL/
  auto to_message(std::vector<std::pair<std::string_view, std::string_view>>& fields) -> std::vector<std::byte>{
    size_t message_size = 0;
    for(auto& [key, value] : fields){
      message_size += key.size() + value.size() + sizeof(uint64_t) + 2; // +2 here for two new lines
    }
    char newline = '\n';
    std::vector<std::byte> ret_value(message_size, {});
    size_t index = 0;
    for(auto& [key, value] : fields){
      uint64_t value_size = value.size();
      memcpy(ret_value.data() + index, key.data(), key.size());
      index += key.size();
      memcpy(ret_value.data() + index, &newline, 1);
      index += 1;
      memcpy(ret_value.data() + index, &value_size, sizeof(value_size));
      index += sizeof(value_size);
      memcpy(ret_value.data() + index, value.data(), value.size());
      index += value.size();
      memcpy(ret_value.data() + index, &newline, 1);
      index += 1;
    }
    assert(index == ret_value.size() && "Buffer allocated of correct size");
    return ret_value;
  }
}