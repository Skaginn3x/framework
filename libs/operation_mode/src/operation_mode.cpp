#include <ranges>

#include <tfc/dbus/match_rules.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/dbus/sdbusplus_meta.hpp>
#include <tfc/operation_mode.hpp>
#include <tfc/stx/concepts.hpp>
#include <tfc/stx/string_view_join.hpp>

#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/property.hpp>
#include <sdbusplus/bus/match.hpp>
#include <sdbusplus/message.hpp>

namespace tfc::operation {

namespace asio = boost::asio;

/*
Type=signal  Endian=l  Flags=1  Version=1 Cookie=5  Timestamp="Tue 2023-05-23 08:18:56.206938 UTC"
Sender=:1.1578  Path=/com/skaginn3x/operation_mode  Interface=com.skaginn3x.operation_mode  Member=update
UniqueName=:1.1578
MESSAGE "(ss)" {
        STRUCT "ss" {
                STRING "starting";
                STRING "stopped";
        };
};
 */

[[maybe_unused]] static constexpr std::string_view mode_update_match_rule{
  stx::string_view_join_v<tfc::dbus::match::rules::type::signal,
                          tfc::dbus::match::rules::member<tfc::operation::dbus::signal::update>,
                          tfc::dbus::match::rules::interface<tfc::operation::dbus::name>,
                          tfc::dbus::match::rules::path<tfc::operation::dbus::path>>
};

interface::interface(asio::io_context& ctx, std::string_view log_key, std::string_view dbus_service_name)
    : dbus_service_name_{ dbus_service_name },
      dbus_connection_{ std::make_unique<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system()) },
      mode_updates_{ std::make_unique<sdbusplus::bus::match_t>(*dbus_connection_,
                                                               mode_update_match_rule.data(),
                                                               std::bind_front(&interface::mode_update, this)) },
      logger_{ log_key } {
  sdbusplus::asio::getProperty<mode_e>(*dbus_connection_, dbus_service_name_, std::string{ dbus::path },
                                       std::string{ dbus::name }, std::string{ dbus::property::mode },
                                       [this](auto err, mode_e const& mode) {
                                         if (err) {
                                           logger_.warn("Error from get mode: {}", err.message());
                                           return;
                                         }
                                         mode_update_impl(update_message{ mode, mode_e::unknown });
                                       });
}

interface::interface(interface&& to_be_erased) noexcept
    : dbus_connection_(std::move(to_be_erased.dbus_connection_)),
      mode_updates_{ std::make_unique<sdbusplus::bus::match_t>(*dbus_connection_,
                                                               mode_update_match_rule.data(),
                                                               std::bind_front(&interface::mode_update, this)) },
      logger_{ std::move(to_be_erased.logger_) } {
  // It is pretty safe to construct new match here it mostly invokes C api where it does not explicitly throw
  // it could throw if we are out of memory but then we are already screwed and the process will terminate.
}

auto interface::operator=(interface&& to_be_erased) noexcept -> interface& {
  dbus_connection_ = std::move(to_be_erased.dbus_connection_);
  // It is pretty safe to construct new match here it mostly invokes C api where it does not explicitly throw
  // it could throw if we are out of memory but then we are already screwed and the process will terminate.
  mode_updates_ = std::make_unique<sdbusplus::bus::match_t>(*dbus_connection_, mode_update_match_rule.data(),
                                                            std::bind_front(&interface::mode_update, this));
  logger_ = std::move(to_be_erased.logger_);
  return *this;
}

void interface::set(tfc::operation::mode_e new_mode) const {
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

void interface::stop(const std::string_view reason) const {
  dbus_connection_->async_method_call(
      [this](std::error_code err) {
        if (err) {
          logger_.warn("Error from stop : {}", err.message());
        }
      },
      dbus_service_name_, std::string{ dbus::path }, std::string{ dbus::name }, std::string{ dbus::method::stop_w_reason },
      std::string(reason));
}

void interface::mode_update(sdbusplus::message::message msg) noexcept {
  mode_update_impl(msg.unpack<update_message>());
}
void interface::mode_update_impl(update_message const update_msg) noexcept {
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

}  // namespace tfc::operation
