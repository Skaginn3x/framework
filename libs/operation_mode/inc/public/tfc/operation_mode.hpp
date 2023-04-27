#pragma once

#include <atomic>
#include <concepts>
#include <cstdint>
#include <functional>
#include <system_error>
#include <vector>

#include <tfc/logger.hpp>

namespace tfc::operation {

enum struct mode_e : std::uint8_t {
  unknown = 0,
  stopped = 1,
  running = 2,
  fault = 3,
  cleaning = 4,
  emergency = 5,
  maintenance = 6,
};
namespace concepts {
using new_mode_e = mode_e;
using old_mode_e = mode_e;
template <typename callback_t>
concept transition_callback = std::invocable<callback_t, new_mode_e, old_mode_e>;
}  // namespace concepts

class interface {
  interface() : logger_("operation") {}
  explicit interface(std::string_view log_key) : logger_(log_key) {}

  /// \brief set operation mode controller to new state
  /// \note take care since this will affect the whole system
  void set(mode_e) const;

  using uuid_t = std::uint64_t;
  using new_mode_e = mode_e;
  using old_mode_e = mode_e;

  std::error_code remove_callback(uuid_t uuid) {
    auto number_of_erased_items{ std::erase_if(callbacks, [uuid](auto const& item) -> bool { return item.uuid == uuid; }) };
    if (number_of_erased_items == 0) {
      return std::make_error_code(std::errc::argument_out_of_domain);
    }
    return {};
  }

  uuid_t on_enter(mode_e mode, concepts::transition_callback auto&& callback) {
    return append_callback(mode, transition_e::enter, std::forward<decltype(callback)>(callback));
  }

  uuid_t on_leave(mode_e mode, concepts::transition_callback auto&& callback) {
    return append_callback(mode, transition_e::leave, std::forward<decltype(callback)>(callback));
  }

  void on_enter_once(mode_e mode, concepts::transition_callback auto&& callback) {
    std::shared_ptr<uuid_t> uuid{};
    uuid = std::make_shared<uuid_t>(on_enter(mode, once_callback_wrapper(std::forward<decltype(callback)>(callback), uuid)));
  }

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

  uuid_t append_callback(mode_e mode_value, transition_e transition, concepts::transition_callback auto&& callback) {
    uuid_t uuid{ next_uuid++ };
    callbacks.emplace_back(callback_item{ .mode = mode_value,
                                          .transition = transition,
                                          .callback = std::forward<decltype(callback)>(callback),
                                          .uuid = uuid });
    return uuid;
  }

  auto once_callback_wrapper(concepts::transition_callback auto&& callback, std::shared_ptr<uuid_t> uuid) {
    return [this, cb = callback, uuid](new_mode_e new_mode, old_mode_e old_mode) {
      cb(new_mode, old_mode);
      if (uuid == nullptr) {
        logger_.warn("Weird error occurred, unable to clean up callback");
        return;
      }
      if (remove_callback(*uuid)) {
        logger_.info("Unable to remove callback from callbacks");
      }
    };
  }

  std::atomic<uuid_t> next_uuid{};
  std::vector<callback_item> callbacks{};
  tfc::logger::logger logger_;
};

}  // namespace tfc::operation
