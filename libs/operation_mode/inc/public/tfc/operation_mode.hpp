#pragma once

import std;
import sdbus;
import asio;

#include <tfc/logger.hpp>
#include <tfc/operation_mode/common.hpp>

namespace tfc::operation {

using new_mode_e = mode_e;
using old_mode_e = mode_e;

namespace concepts {

template <typename callback_t>
concept transition_callback = std::invocable<callback_t, new_mode_e, old_mode_e>;
}  // namespace concepts

class interface {
public:
  explicit interface(asio::io_context& ctx) : interface(ctx, "operation") {}
  interface(asio::io_context& ctx, std::string_view log_key) : interface(ctx, log_key, dbus::service_name) {}
  /// \brief construct an interface to operation mode controller
  /// \param ctx context to run in
  /// \param log_key key to use for logging
  /// \param dbus_service_name name of the service to connect to
  interface(asio::io_context& ctx, std::string_view log_key, std::string_view dbus_service_name);
  interface(interface const&) = delete;
  auto operator=(interface const&) -> interface& = delete;
  interface(interface&&) noexcept;
  auto operator=(interface&&) noexcept -> interface&;

  /// \brief set operation mode controller to new state
  /// \note take care since this will affect the whole system
  void set(mode_e) const;

  using uuid_t = std::uint64_t;
  using new_mode_e = mode_e;
  using old_mode_e = mode_e;

  /// \brief remove callback subscription
  /// \param uuid given id from return value of subscription
  auto remove_callback(uuid_t uuid) -> std::error_code {
    auto number_of_erased_items{ std::erase_if(callbacks_, [uuid](auto const& item) -> bool { return item.uuid == uuid; }) };
    if (number_of_erased_items == 0) {
      return std::make_error_code(std::errc::argument_out_of_domain);
    }
    return {};
  }

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
  enum struct transition_e : std::uint8_t {
    unknown = 0,
    enter,
    leave,
  };

  struct callback_item {
    mode_e mode{ mode_e::unknown };
    transition_e transition{ transition_e::unknown };
    std::function<void(new_mode_e, old_mode_e)> callback{};
    uuid_t uuid{};
  };

  auto append_callback(mode_e mode_value, transition_e transition, concepts::transition_callback auto&& callback) -> uuid_t {
    uuid_t const uuid{ next_uuid_++ };
    callbacks_.emplace_back(callback_item{ .mode = mode_value,
                                           .transition = transition,
                                           .callback = std::forward<decltype(callback)>(callback),
                                           .uuid = uuid });
    return uuid;
  }

  auto once_callback_wrapper(concepts::transition_callback auto&& callback, std::shared_ptr<uuid_t> uuid) {
    return [this, cb = callback, copy_uuid = uuid](new_mode_e new_mode, old_mode_e old_mode) {
      cb(new_mode, old_mode);
      if (copy_uuid == nullptr) {
        logger_.warn("Weird error occurred, unable to clean up callback");
        return;
      }
      if (remove_callback(*copy_uuid)) {
        logger_.info("Unable to remove callback from callbacks");
      }
    };
  }

  void mode_update(sdbusplus::message::message) noexcept;
  void mode_update_impl(update_message) noexcept;

  uuid_t next_uuid_{};
  std::string dbus_service_name_{};
  std::vector<callback_item> callbacks_{};
  std::unique_ptr<sdbusplus::asio::connection, std::function<void(sdbusplus::asio::connection*)>> dbus_connection_{};
  std::unique_ptr<sdbusplus::bus::match::match, std::function<void(sdbusplus::bus::match::match*)>> mode_updates_{};
  tfc::logger::logger logger_;
};

}  // namespace tfc::operation
