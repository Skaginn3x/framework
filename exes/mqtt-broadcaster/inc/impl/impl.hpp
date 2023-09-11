#pragma once

#include <any>
#include <optional>

namespace tfc::mqtt::impl {

struct signal_data {
  tfc::ipc_ruler::signal information;
  tfc::ipc::details::any_recv receiver;
  std::optional<std::any> current_value;
};

auto topic_formatter(const std::vector<std::string_view>& topic_vector, std::string& topic_storage) -> std::string_view {
  if (topic_vector.empty()) {
    return std::string_view();
  }

  topic_storage.clear();  // Clear any existing data
  for (const auto& sub_topic : topic_vector) {
    topic_storage += std::string(sub_topic) + "/";
  }
  topic_storage.pop_back();  // Remove the last '/'

  return std::string_view(topic_storage);
}

}  // namespace tfc::mqtt::impl
