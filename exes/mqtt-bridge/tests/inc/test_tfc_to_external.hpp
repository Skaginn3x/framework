#pragma once

import std;
import asio;

namespace tfc::mqtt {

class test_tfc_to_external {
public:
  explicit test_tfc_to_external() = default;

  static auto run_io_context_for(asio::io_context& ctx, std::chrono::milliseconds duration) -> void;

  static auto check_value(const std::optional<std::any>& value, bool expected_value) -> bool;

  static auto test() -> asio::awaitable<bool>;
};

}  // namespace tfc::mqtt
