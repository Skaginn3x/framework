#pragma once

#include <cstdint>
#include <memory>
#include <functional>
#include <expected>

struct ec_master;
using ec_master_t = ec_master;

namespace tfc::ec {
class domain;

class master {
public:
  using master_t = std::unique_ptr<ec_master_t, std::function<void(ec_master_t*)>>;
  static auto request(uint32_t const& master_index) -> void; //std::expected<master, std::error_code>;

  auto create_domain() -> void;

private:
  explicit master(master_t&& master_created) : master_{std::move(master_created)}{};
  master_t master_;
};

class domain {

};
}  // namespace tfc::ec