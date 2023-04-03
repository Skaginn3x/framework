#pragma once

#include <boost/asio/io_context.hpp>

#include "tfc/logger.hpp"

#include "tfc/ec/soem_interface.hpp"

PRAGMA_CLANG_WARNING_PUSH_OFF(-Wweak-vtables)
namespace tfc::ec::devices {
class base {
public:
  virtual ~base() = default;
  // Default behaviour no data processing
  virtual void process_data(std::span<std::byte>, std::span<std::byte>) = 0;
  // Default behaviour, no setup
  virtual auto setup(ecx_contextt*, uint16_t) -> int { return 1; }

protected:
  explicit base(uint16_t slave_index) : slave_index_(slave_index), logger_(fmt::format("Ethercat slave {}", slave_index)) {}
  const uint16_t slave_index_;
  tfc::logger::logger logger_;
};

class default_device : public base {
public:
  explicit default_device(uint16_t const slave_index) : base(slave_index) {}
  void process_data(std::span<std::byte>, std::span<std::byte>) noexcept override{}
  auto setup(ecx_contextt*, uint16_t) -> int override { return 1; }
};
}  // namespace tfc::ec::devices
PRAGMA_CLANG_WARNING_POP
