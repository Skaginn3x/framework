#pragma once

#include <system_error>

#include <tfc/ipc/details/dbus_server_iface.hpp>
#include <tfc/ipc/details/dbus_server_iface_mock.hpp>
#include <tfc/ipc/details/filter.hpp>
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

/// \brief make manager client for the types(signal, slot) below
[[maybe_unused]] static auto make_manager_client(asio::io_context& ctx) {
  return tfc::ipc_ruler::ipc_manager_client{ ctx };
}

/**
 * @brief
 * This is the receiving end for tfc's ipc communications,
 * it listens to connection changes from ipc-ruler in
 * addition with implementing the ipc connection.
 * @tparam type_desc The type description for the slot.
 */
template <typename type_desc, typename manager_client_type = tfc::ipc_ruler::ipc_manager_client>
class slot {
public:
  using value_t = typename details::slot_callback<type_desc>::value_t;
  static constexpr std::string_view type_name{ type_desc::type_name };
  static auto constexpr direction_v = details::slot_callback<type_desc>::direction_v;

  /**
   * C'tor for a tfc IPC slot.
   * @param ctx Execution context
   * @param client manager_client_type a reference to a manager client
   * @param name The slot name
   * @param callback Channel for value updates from the corosponding signal.
   */
  slot(asio::io_context& ctx,
       manager_client_type& client,
       std::string_view name,
       std::string_view description,
       std::invocable<value_t> auto&& callback)
      : slot_(details::slot_callback<type_desc>::create(ctx, name)), client_(client),
        filters_{ ctx, name, std::forward<decltype(callback)>(callback) } {
    client_.register_connection_change_callback(
        slot_->name_w_type(), [this](const std::string_view signal_name) { slot_->init(signal_name, filters_); });
    client_.register_slot(slot_->name_w_type(), description, type_desc::value_e, details::register_cb(slot_->name_w_type()));
  }

  slot(asio::io_context& ctx, manager_client_type& client, std::string_view name, auto&& callback)
      : slot(ctx, client, name, "", callback) {}

  slot(slot&) = delete;

  slot(slot&&) noexcept = delete;

  auto operator=(slot&&) noexcept -> slot& = delete;

  auto operator=(slot const&) -> slot& = delete;

  [[nodiscard]] auto value() const noexcept { return slot_->get(); }  // todo: value() or get()

private:
  static constexpr std::string_view self_name{ slot_tag };
  std::shared_ptr<details::slot_callback<type_desc>> slot_;
  manager_client_type& client_;
  filter::filters<value_t, std::function<void(value_t&)>> filters_;  // todo prefer some other type erasure mechanism
};

/**
 * an ipc component, registers its existence with an
 * ipc-ruler service.
 * @tparam type_desc The type of the signal
 */
template <typename type_desc, typename manager_client_type>
class signal {
public:
  using value_t = typename details::signal<type_desc>::value_t;
  static constexpr std::string_view type_name{ type_desc::type_name };
  static auto constexpr direction_v = details::signal<type_desc>::direction_v;

  /**
   * Signal c'tor
   * @param ctx Execution context
   * @param name Signals name
   */
  signal(asio::io_context& ctx, manager_client_type& client, std::string_view name, std::string_view description = "")
      : client_(client) {
    auto exp{ details::signal<type_desc>::create(ctx, name) };
    if (!exp.has_value()) {
      throw std::runtime_error{ fmt::format("Unable to bind to socket, reason: {}", exp.error().message()) };
    }
    signal_ = std::move(exp.value());
    client_.register_signal(signal_->name_w_type(), description, type_desc::value_e,
                            details::register_cb(signal_->name_w_type()));
  }

  auto send(value_t const& value) -> std::error_code { return signal_->send(value); }

  template <typename CompletionToken>
  auto async_send(value_t const& value, CompletionToken&& token) -> auto {
    return signal_->async_send(value, std::forward<decltype(token)>(token));
  }

  auto name() const noexcept -> std::string_view { return signal_->name(); }

private:
  static constexpr std::string_view self_name{ signal_tag };
  manager_client_type& client_;
  std::shared_ptr<details::signal<type_desc>> signal_;
};

using bool_slot = slot<details::type_bool, ipc_ruler::ipc_manager_client>;
using int_slot = slot<details::type_int, ipc_ruler::ipc_manager_client>;
using uint_slot = slot<details::type_uint, ipc_ruler::ipc_manager_client>;
using double_slot = slot<details::type_double, ipc_ruler::ipc_manager_client>;
using string_slot = slot<details::type_string, ipc_ruler::ipc_manager_client>;
using json_slot = slot<details::type_json, ipc_ruler::ipc_manager_client>;

using bool_signal = signal<details::type_bool, ipc_ruler::ipc_manager_client>;
using int_signal = signal<details::type_int, ipc_ruler::ipc_manager_client>;
using uint_signal = signal<details::type_uint, ipc_ruler::ipc_manager_client>;
using double_signal = signal<details::type_double, ipc_ruler::ipc_manager_client>;
using string_signal = signal<details::type_string, ipc_ruler::ipc_manager_client>;
using json_signal = signal<details::type_json, ipc_ruler::ipc_manager_client>;
using any_signal = std::variant<bool_signal, int_signal, uint_signal, double_signal, string_signal, json_signal>;

}  // namespace tfc::ipc
