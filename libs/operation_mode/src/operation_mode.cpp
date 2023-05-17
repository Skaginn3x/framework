#include <ranges>

#include <tfc/dbus/match_rules.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/dbus/sdbusplus_meta.hpp>
#include <tfc/operation_mode.hpp>
#include <tfc/stx/concepts.hpp>
#include <tfc/stx/string_view_join.hpp>

#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/bus/match.hpp>
#include <sdbusplus/message.hpp>

namespace tfc::operation {

namespace asio = boost::asio;

[[maybe_unused]] static constexpr std::string_view mode_update_match_rule{
  stx::string_view_join_v<tfc::dbus::match::rules::type::signal,
                          tfc::dbus::match::rules::sender<tfc::operation::dbus::signal::update> >
};

interface::interface(asio::io_context& ctx, std::string_view log_key)
    : dbus_connection_{ std::make_unique<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system()) },
      mode_updates_{ std::make_unique<sdbusplus::bus::match_t>(*dbus_connection_,
                                                               mode_update_match_rule.data(),
                                                               std::bind_front(&interface::mode_update, this)) },
      logger_{ log_key } {}

void interface::set(tfc::operation::mode_e new_mode) const {
  // todo add handler to set function call, for callee
  dbus_connection_->async_method_call([](std::error_code) {}, dbus::name.data(), dbus::path.data(), dbus::name.data(),
                                      dbus::method::set_mode.data(), new_mode);
}

void interface::mode_update(sdbusplus::message::message& msg) noexcept {
  auto update_msg = msg.unpack<update_message>();

  static constexpr auto make_transition_filter{ [](transition_e trans) noexcept {
    return [trans](callback_item const& itm) { return itm.transition == trans; };
  } };
  static constexpr auto make_mode_filter{ [](mode_e mode) noexcept {
    return [mode](callback_item const& itm) { return itm.mode == mode; };
  } };

  static const auto invoke{ [this, update_msg](callback_item const& itm) noexcept {
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

}  // namespace tfc::operation
