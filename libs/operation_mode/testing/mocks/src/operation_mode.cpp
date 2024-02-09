#include "../inc/public/tfc/mocks/operation_mode.hpp"


namespace {
  using uuid_t = std::uint64_t;
  thread_local uuid_t next_uuid_;
}

namespace tfc::operation {
mock_interface::mock_interface(asio::io_context& ctx, std::string_view log_key, std::string_view) {
}

mock_interface::mock_interface(mock_interface&& to_be_erased) noexcept : logger_{ std::move(to_be_erased.logger_) } {
}

auto mock_interface::operator=(mock_interface&& to_be_erased) noexcept -> mock_interface& {
  logger_ = std::move(to_be_erased.logger_);
  return *this;
}

void mock_interface::set(tfc::operation::mode_e new_mode) const {
  // todo add handler to set function call, for callee
  dbus_connection_->async_method_call(
      [this](std::error_code err) {
        if (err) {
          logger_.warn("Error from set mode: {}", err.message());
        }
      },
      dbus_service_name_, std::string{ dbus::path }, std::string{ dbus::name }, std::string{ dbus::method::set_mode },
      new_mode);
}

void mock_interface::stop(const std::string_view reason) const {
  dbus_connection_->async_method_call(
      [this](std::error_code err) {
        if (err) {
          logger_.warn("Error from stop : {}", err.message());
        }
      },
      dbus_service_name_, std::string{ dbus::path }, std::string{ dbus::name }, std::string{ dbus::method::stop_w_reason },
      std::string(reason));
}

void mock_interface::mode_update(sdbusplus::message::message msg) noexcept {
  mode_update_impl(msg.unpack<update_message>());
}
void mock_interface::mode_update_impl(update_message const update_msg) noexcept {
  constexpr auto make_transition_filter{ [](transition_e trans) noexcept {
    return [trans](callback_item const& itm) { return itm.transition == trans; };
  } };
  constexpr auto make_mode_filter{ [](mode_e mode) noexcept {
    return [mode](callback_item const& itm) { return itm.mode == mode; };
  } };

  const auto invoke{ [this, update_msg](callback_item const& itm) noexcept {
    if (itm.callback) {
      try {
        std::invoke(itm.callback, update_msg.new_mode, update_msg.old_mode);
      } catch ([[maybe_unused]] std::exception const& exc) {
        logger_.warn(R"(Exception from callback id: "{}", what: "{}")", itm.uuid, exc.what());
      }
    }
  } };

  // Call leave first
  for (auto& callback : callbacks_ | std::views::filter(make_transition_filter(transition_e::leave)) |
                            std::views::filter(make_mode_filter(update_msg.old_mode))) {
    invoke(callback);
  }
  // Call enter second
  for (auto& callback : callbacks_ | std::views::filter(make_transition_filter(transition_e::enter)) |
                            std::views::filter(make_mode_filter(update_msg.new_mode))) {
    invoke(callback);
  }
}
}
