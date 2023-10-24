#pragma once

#include <tfc/utils/asio_fwd.hpp>

namespace tfc::mqtt {

namespace asio = boost::asio;

class test_external_to_tfc {
public:
  explicit test_external_to_tfc() = default;

  static auto test() -> bool;
};
}  // namespace tfc::mqtt
