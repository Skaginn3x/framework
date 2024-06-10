#pragma once

#include <memory>
#include <utility>
#include <variant>

#include <fmt/core.h>

#include "abt/easycat.hpp"
#include "beckhoff/EK1xxx.hpp"
#include "beckhoff/EL1xxx.hpp"
#include "beckhoff/EL2xxx.hpp"
#include "beckhoff/EL3xxx.hpp"
#include "beckhoff/EL4xxx.hpp"
#include "beckhoff/EQ2339.hpp"
#include "eilersen/4x60a.hpp"
#include "schneider/atv320.hpp"
#include "schneider/lxm32m.hpp"

namespace tfc::ec::devices {

template <typename...>
struct devices_type {};

// clang-format off
template <typename manager_client_t>
using devices_list = devices_type<
  abt::easyecat<manager_client_t>,
  beckhoff::ek1100,
  beckhoff::el1002<manager_client_t>,
  beckhoff::el1008<manager_client_t>,
  beckhoff::el1809<manager_client_t>,
  beckhoff::el2794<manager_client_t>,
  beckhoff::el2004<manager_client_t>,
  beckhoff::el2008<manager_client_t>,
  beckhoff::el2809<manager_client_t>,
  beckhoff::eq2339<manager_client_t>,
  beckhoff::el3054,
  beckhoff::el4002,
  schneider::atv320::device<manager_client_t>,
  schneider::lxm32m<manager_client_t>,
  eilersen::e4x60a::e4x60a<manager_client_t>
>;
// clang-format on

// todo appendable type list
// https://b.atch.se/posts/constexpr-meta-container/
// https://mc-deltat.github.io/articles/stateful-metaprogramming-cpp20

template <typename manager_client_t>
using device_variant = std::variant<abt::easyecat<manager_client_t>,
                                    beckhoff::ek1100,
                                    beckhoff::el1002<manager_client_t>,
                                    beckhoff::el1008<manager_client_t>,
                                    beckhoff::el1809<manager_client_t>,
                                    beckhoff::el2794<manager_client_t>,
                                    beckhoff::el2004<manager_client_t>,
                                    beckhoff::el2008<manager_client_t>,
                                    beckhoff::el2809<manager_client_t>,
                                    beckhoff::eq2339<manager_client_t>,
                                    beckhoff::el3054,
                                    beckhoff::el4002,
                                    schneider::atv320::device<manager_client_t>,
                                    schneider::lxm32m<manager_client_t>,
                                    eilersen::e4x60a::e4x60a<manager_client_t>,
                                    default_device>;
template <typename manager_client_t>
struct device {
  template <typename in_place_type_t, typename... args_t>
  explicit device(in_place_type_t&& in_place_type, args_t&&... args)
      : device_{ std::make_unique<device_variant<manager_client_t>>(in_place_type, std::forward<args_t>(args)...) } {}
  device(device const&) = delete;
  device(device&&) noexcept = default;
  auto operator=(device const&) -> device& = delete;
  auto operator=(device&&) noexcept -> device& = default;
  ~device() = default;

  void set_sdo_write_cb(
      std::function<
          ecx::working_counter_t(ecx::index_t, ecx::complete_access_t, std::span<std::byte>, std::chrono::microseconds)>
          cb) {
    std::visit([cb](auto& impl) { impl.set_sdo_write_cb(cb); }, *device_);
  }

  void process_data(std::span<std::uint8_t> input, std::span<std::uint8_t> output) {
    std::visit([input, output](auto& impl) { impl.process_data(input, output); }, *device_);
  }
  auto setup() -> int {
    return std::visit([](auto& impl) { return impl.setup(); }, *device_);
  }
  // unique_ptr to make this struct movable with non movable items
  std::unique_ptr<device_variant<manager_client_t>> device_{};
};

template <typename device>
auto devices_equal(auto vendor_id, auto product_code) {
  return vendor_id == device::vendor_id && product_code == device::product_code;
}

template <typename manager_client_type, typename device_t>
auto get_impl(std::shared_ptr<sdbusplus::asio::connection>& connection,
              manager_client_type& client,
              uint16_t const slave_index,
              auto vendor_id,
              auto product_code,
              std::optional<device<manager_client_type>>& output) -> bool {
  if (devices_equal<device_t>(vendor_id, product_code)) {
    if constexpr (std::is_constructible_v<device_t, std::shared_ptr<sdbusplus::asio::connection>, manager_client_type&,
                                          uint16_t>) {
      output =
          std::make_optional<device<manager_client_type>>(std::in_place_type<device_t>, connection, client, slave_index);
    } else if constexpr (std::is_constructible_v<device_t, boost::asio::io_context&, manager_client_type&, uint16_t>) {
      output = std::make_optional<device<manager_client_type>>(std::in_place_type<device_t>, connection->get_io_context(),
                                                               client, slave_index);
    } else if constexpr (std::is_constructible_v<device_t, boost::asio::io_context&, uint16_t>) {
      output = std::make_optional<device<manager_client_type>>(std::in_place_type<device_t>, connection->get_io_context(),
                                                               slave_index);
    } else if constexpr (std::is_constructible_v<device_t, uint16_t>) {
      output = std::make_optional<device<manager_client_type>>(std::in_place_type<device_t>, slave_index);
    } else {
      // clang-format off
      []<bool flag = false>() { static_assert(flag, "No matching constructor"); } ();
      // clang-format on
    }
    return true;
  }
  return false;
}

template <typename manager_client_type, typename... devices_t>
auto get_impl(std::shared_ptr<sdbusplus::asio::connection>& connection,
              manager_client_type& client,
              uint16_t const slave_index,
              auto vendor_id,
              auto product_code,
              devices_type<devices_t...>) -> device<manager_client_type> {
  std::optional<device<manager_client_type>> result{};
  (get_impl<manager_client_type, devices_t>(connection, client, slave_index, vendor_id, product_code, result) || ...);
  if (result) {
    return std::move(result.value());
  }
  tfc::logger::logger log("missing_implementation");
  log.warn("No device found for slave: {}, vendor: {}, product_code: {} \n", slave_index, vendor_id, product_code);
  return device<manager_client_type>(std::in_place_type<default_device>, slave_index);
}

template <typename manager_client_type>
auto get(std::shared_ptr<sdbusplus::asio::connection>& connection,
         manager_client_type& client,
         uint16_t const slave_index,
         auto vendor_id,
         auto product_code) -> device<manager_client_type> {
  return get_impl(connection, client, slave_index, vendor_id, product_code, devices_list<manager_client_type>{});
}
}  // namespace tfc::ec::devices
