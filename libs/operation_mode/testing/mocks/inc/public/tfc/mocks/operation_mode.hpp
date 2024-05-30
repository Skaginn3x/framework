#pragma once
#include <functional>
#include <string_view>
#include <system_error>

#include <gmock/gmock.h>
#include <boost/asio/io_context.hpp>

#include <tfc/operation_mode.hpp>

namespace tfc::operation {

namespace asio = boost::asio;

struct mock_interface {
  explicit mock_interface(asio::io_context& ctx) : mock_interface(ctx, "operation") {}
  mock_interface(asio::io_context& ctx, std::string_view log_key) : mock_interface(ctx, log_key, dbus::service_name) {}
  /// \brief construct an interface to operation mode controller
  /// \param ctx context to run in
  /// \param log_key key to use for logging
  /// \param dbus_service_name name of the service to connect to
  mock_interface(asio::io_context& ctx, std::string_view log_key, std::string_view dbus_service_name);
  mock_interface(mock_interface const&) = delete;
  auto operator=(mock_interface const&) -> mock_interface& = delete;
  mock_interface(mock_interface&&) noexcept;
  auto operator=(mock_interface&&) noexcept -> mock_interface&;

  /// \brief set operation mode controller to new state
  /// \note take care since this will affect the whole system
  void set(mode_e) const;

  /// \brief stop operation mode controller
  /// \param reason reason for stopping
  /// \note the reason can be very useful if components are stopping due to an error.
  void stop(const std::string_view reason) const;

  using new_mode_e = mode_e;
  using old_mode_e = mode_e;

  /// \brief remove callback subscription
  /// \param uuid given id from return value of subscription
  auto remove_callback(uuid_t uuid) -> std::error_code;

  /// \brief remove all callback subscriptions
  auto remove_all_callbacks() -> std::error_code;

  /// \brief reset operation mode to starting state
  auto reset() -> std::error_code;

  /// \brief subscribe to events when entering new mode
  /// \param mode mode to subscribe to
  /// \param callback invocable<new_mode_e, old_mode_e>
  auto on_enter(mode_e mode, concepts::transition_callback auto&& callback) -> uuid_t {
    return append_callback(mode, transition_e::enter, std::forward<decltype(callback)>(callback));
  }

  /// \brief subscribe to events when leaving a given mode
  /// \param mode leaving this mode
  /// \param callback invocable<new_mode_e, old_mode_e>
  auto on_leave(mode_e mode, concepts::transition_callback auto&& callback) -> uuid_t {
    return append_callback(mode, transition_e::leave, std::forward<decltype(callback)>(callback));
  }

  /// \brief subscribe to events when entering new mode
  /// \param mode mode to subscribe to
  /// \param callback invocable<new_mode_e, old_mode_e>
  /// \note will automatically remove the subscription after it is called
  void on_enter_once(mode_e mode, concepts::transition_callback auto&& callback) {
    std::shared_ptr<uuid_t> uuid{};
    uuid = std::make_shared<uuid_t>(on_enter(mode, once_callback_wrapper(std::forward<decltype(callback)>(callback), uuid)));
  }

  /// \brief subscribe to events when leaving a given mode
  /// \param mode leaving this mode
  /// \param callback invocable<new_mode_e, old_mode_e>
  /// \note will automatically remove the subscription after it is called
  void on_leave_once(mode_e mode, concepts::transition_callback auto&& callback) {
    std::shared_ptr<uuid_t> uuid{};
    uuid = std::make_shared<uuid_t>(on_leave(mode, once_callback_wrapper(std::forward<decltype(callback)>(callback), uuid)));
  }

private:
  auto get_next_uuid() -> uuid_t;

  auto append_callback_impl(mode_e mode_value, transition_e transition, std::function<void(new_mode_e, old_mode_e)> callback)
      -> uuid_t;
  auto append_callback(mode_e mode_value, transition_e transition, concepts::transition_callback auto&& callback) -> uuid_t {
    return append_callback_impl(mode_value, transition, std::forward<decltype(callback)>(callback));
  }

  void mode_update_impl(update_message) noexcept;
};

}  // namespace tfc::operation
