#pragma once

#include <any>
#include <optional>

namespace tfc::mqtt::impl {

struct signal_data {
  tfc::ipc_ruler::signal information;
  tfc::ipc::details::any_recv receiver;
  std::optional<std::any> current_value;
};

// TODO: Make this use std::string_view
auto topic_formatter(const std::vector<std::string_view>& topic_vector) -> std::string {
  if (topic_vector.empty()) {
    throw std::runtime_error("Topic can not be empty");
  }
  std::string topic;
  for (const auto& sub_topic : topic_vector) {
    topic += std::string(sub_topic) + "/";
  }
  topic.pop_back();
  return topic;
}
}  // namespace tfc::mqtt::impl
