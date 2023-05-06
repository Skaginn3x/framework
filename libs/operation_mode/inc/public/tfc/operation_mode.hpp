#pragma once

#include <concepts>
#include <cstdint>
#include <functional>
#include <system_error>
#include <vector>

#include <tfc/logger.hpp>
#include <tfc/operation_mode/common.hpp>

namespace sdbusplus::asio {
class connection;
}
namespace sdbusplus::bus::match {
struct match;
}
namespace sdbusplus::message {
class message;
}
namespace boost::asio {
class io_context;
}

namespace tfc::operation {

using new_mode_e = mode_e;
using old_mode_e = mode_e;

namespace concepts {

template <typename callback_t>
concept transition_callback = std::invocable<callback_t, new_mode_e, old_mode_e>;
}  // namespace concepts

class interface {
public:
  explicit interface(boost::asio::io_context& ctx) : interface(ctx, "operation") {}
  interface(boost::asio::io_context& ctx, std::string_view log_key);

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

  void mode_update(sdbusplus::message::message&) noexcept;

  uuid_t next_uuid_{};
  std::vector<callback_item> callbacks_{};
  std::unique_ptr<sdbusplus::asio::connection, std::function<void(sdbusplus::asio::connection*)>> dbus_connection_{};
  std::unique_ptr<sdbusplus::bus::match::match, std::function<void(sdbusplus::bus::match::match*)>> mode_updates_{};
  tfc::logger::logger logger_;
};

}  // namespace tfc::operation
