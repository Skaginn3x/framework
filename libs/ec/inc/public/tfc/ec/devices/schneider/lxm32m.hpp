#pragma once
// https://download.schneider-electric.com/files?p_enDocType=User+guide&p_File_Name=0198441113868_06.pdf&p_Doc_Ref=0198441113868-EN

#include <boost/asio/io_context.hpp>
#include <glaze/glaze.hpp>

#include <tfc/confman.hpp>

#include "tfc/cia/402.hpp"
#include "tfc/ec/devices/base.hpp"

namespace tfc::ec::devices::schneider::lxm32 {

enum struct operation_mode_e : std::uint8_t { unknown = 0, position, velocity, torque };

struct rx_pdo_position {};

}  // namespace tfc::ec::devices::schneider::lxm32

template <>
struct glz::meta<tfc::ec::devices::schneider::lxm32::operation_mode_e> {
  using enum tfc::ec::devices::schneider::lxm32::operation_mode_e;
  // clang-format off
  static constexpr auto value{ glz::enumerate("unknown", unknown,
                                              "position", position,
                                              "velocity", velocity,
                                              "torque", torque) };
  // clang-format on
  static constexpr auto name{ "lxm32m_operation_mode" };
};

namespace tfc::ec::devices::schneider {

namespace asio = boost::asio;

template <typename manager_client_type>
class lxm32m final : public base {
public:
  static constexpr uint32_t vendor_id = 0x800005a;
  static constexpr uint32_t product_code = 0x16440;

  explicit lxm32m(asio::io_context& ctx, manager_client_type& client, uint16_t slave_index) : base(slave_index) {}

  void process_data(std::span<std::byte>, std::span<std::byte>) final {}
};

}  // namespace tfc::ec::devices::schneider
