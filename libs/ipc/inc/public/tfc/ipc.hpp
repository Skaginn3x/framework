#pragma once

#include <system_error>

#include <tfc/ipc/details/dbus_server_iface.hpp>
#include <tfc/ipc/details/impl.hpp>

namespace tfc::ipc {

namespace asio = boost::asio;

namespace details {
auto constexpr register_cb(std::string const& ipc_name) {
  return [ipc_name](std::error_code error_code) {
    if (error_code == std::errc::host_unreachable) {
      throw std::runtime_error(fmt::format("Error registering: '{}', error: '{}'. Maybe forgot to run ipc-ruler", ipc_name,
                                           error_code.message()));
    }
    if (error_code) {
      throw std::runtime_error(fmt::format("Error registering: '{}', error: '{}'", ipc_name, error_code.message()));
    }
  };
}
}  // namespace details

/**
 * @brief
 * This is the receiving end for tfc's ipc communications,
 * it listens to connection changes from ipc-ruler in
 * addition with implementing the ipc connection.
 * @tparam type_desc The type description for the slot.
 */
template <typename type_desc>
class slot {
public:
  using value_t = details::slot_callback<type_desc>::value_t;
  static constexpr std::string_view type_name{ type_desc::type_name };
  static auto constexpr direction_v = details::slot_callback<type_desc>::direction_v;

  /**
   * C'tor for a tfc IPC slot.
   * @param ctx Execution context
   * @param name The slot name
   * @param callback Channel for value updates from the corosponding signal.
   */
  slot(asio::io_context& ctx, std::string_view name, auto&& callback)
      : slot_(details::slot_callback<type_desc>::create(ctx, name)), client_(ctx) {
    client_.register_connection_change_callback(
        [this, callback](const std::string_view slot_name, const std::string_view signal_name) {
          if (slot_name == slot_->name_w_type()) {
            slot_->init(signal_name, callback);
          }
        });
    client_.register_slot(slot_->name_w_type(), type_desc::value_e, details::register_cb(slot_->name_w_type()));
  }

  slot(slot&) noexcept = default;
  slot(slot&&) noexcept = default;
  auto operator=(slot&&) noexcept -> slot& = default;
  auto operator=(slot const&) noexcept -> slot& = default;

  [[nodiscard]] auto value() const noexcept { return slot_->get(); }  // todo: value() or get()

private:
  static constexpr std::string_view self_name{ slot_tag };
  std::shared_ptr<details::slot_callback<type_desc>> slot_;
  ipc_ruler::ipc_manager_client client_;
};

/**
 * an ipc component, registers its existence with an
 * ipc-ruler service.
 * @tparam type_desc The type of the signal
 */
template <typename type_desc>
class signal {
public:
  using value_t = details::signal<type_desc>::value_t;
  static constexpr std::string_view type_name{ type_desc::type_name };
  static auto constexpr direction_v = details::signal<type_desc>::direction_v;

  /**
   * Signal c'tor
   * @param ctx Execution context
   * @param name Signals name
   */
  signal(asio::io_context& ctx, std::string_view name)
      : signal_{ details::signal<type_desc>::create(ctx, name).value() }, client_(ctx) {
    client_.register_signal(signal_->name_w_type(), type_desc::value_e, details::register_cb(signal_->name_w_type()));
  }
  auto send(value_t const& value) -> std::error_code { return signal_->send(value); }

  auto async_send(value_t const& value, std::invocable<std::error_code, std::size_t> auto&& callback) -> void {
    signal_->async_send(value, std::forward<decltype(callback)>(callback));
  }

private:
  static constexpr std::string_view self_name{ signal_tag };
  std::shared_ptr<details::signal<type_desc>> signal_;
  ipc_ruler::ipc_manager_client client_;
};

using bool_slot = slot<details::type_bool>;
using int_slot = slot<details::type_int>;
using uint_slot = slot<details::type_uint>;
using double_slot = slot<details::type_double>;
using string_slot = slot<details::type_string>;
using json_slot = slot<details::type_json>;

using bool_signal = signal<details::type_bool>;
using int_signal = signal<details::type_int>;
using uint_signal = signal<details::type_uint>;
using double_signal = signal<details::type_double>;
using string_signal = signal<details::type_string>;
using json_signal = signal<details::type_json>;

}  // namespace tfc::ipc
