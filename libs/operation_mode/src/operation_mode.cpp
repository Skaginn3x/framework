#include <ranges>

#include <tfc/operation_mode.hpp>
#include <tfc/stx/concepts.hpp>
#include <tfc/stx/string_view_join.hpp>
#include <tfc/utils/dbus.hpp>

#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/bus/match.hpp>
#include <sdbusplus/message.hpp>

namespace asio = boost::asio;

namespace sdbusplus::message::types::details {
template <tfc::stx::is_enum enum_t>
struct type_id<enum_t> : type_id<std::underlying_type_t<enum_t>> {};

template <>
struct type_id<tfc::operation::update_message> {
  static constexpr auto value = std::tuple_cat(tuple_type_id_v<SD_BUS_TYPE_STRUCT_BEGIN>,
                                               type_id_v<tfc::operation::mode_e>,
                                               type_id_v<tfc::operation::mode_e>,  // todo generate type list
                                               tuple_type_id_v<SD_BUS_TYPE_STRUCT_END>);
};
}  // namespace sdbusplus::message::types::details

namespace sdbusplus::message::details {

template <>
struct read_single<tfc::operation::mode_e> {
  static void op(sdbusplus::SdBusInterface* intf, sd_bus_message* msg, auto& return_value) {
    using underlying_t = std::underlying_type_t<tfc::operation::mode_e>;
    constexpr auto dbus_variable_type = std::get<0>(types::type_id<underlying_t>());
    tfc::operation::mode_e buffer{};
    int const res{ intf->sd_bus_message_read_basic(msg, dbus_variable_type, &buffer) };
    if (res < 0) {
      throw exception::SdBusError(-res, "sd_bus_message_read_basic tfc::operation::mode_e");
    }
    return_value = buffer;
  }
};

template <>
struct read_single<tfc::operation::update_message> {
  static void op(sdbusplus::SdBusInterface* intf, sd_bus_message* msg, auto& return_value) {
    constexpr auto dbus_variable_type = utility::tuple_to_array(std::tuple_cat(
        types::type_id_nonull<tfc::operation::update_message>(), std::make_tuple('\0') /* null terminator for C-string */));

    int res = intf->sd_bus_message_enter_container(msg, SD_BUS_TYPE_STRUCT, dbus_variable_type.data());
    if (res < 0) {
      throw exception::SdBusError(-res, "sd_bus_message_enter_container tfc::operation::update_message");
    }

    tfc::operation::update_message ret{};
    sdbusplus::message::read(intf, msg, ret.new_mode, ret.old_mode);

    res = intf->sd_bus_message_exit_container(msg);
    if (res < 0) {
      throw exception::SdBusError(-res, "sd_bus_message_exit_container tfc::operation::update_message");
    }
    return_value = ret;
  }
};

}  // namespace sdbusplus::message::details

namespace tfc::operation {

static constexpr std::string_view mode_update_match_rule{
  stx::string_view_join_v<tfc::dbus::match::rules::type::signal,
                          tfc::dbus::match::rules::sender<tfc::operation::sender::update>>
};

interface::interface(asio::io_context& ctx) : interface(ctx, "operation") {}
interface::interface(asio::io_context& ctx, std::string_view log_key)
    : dbus_connection_{ new sdbusplus::asio::connection(ctx), [](sdbusplus::asio::connection* conn) { delete conn; } },
      mode_updates_{ new sdbusplus::bus::match_t(*dbus_connection_,
                                                 mode_update_match_rule.data(),
                                                 std::bind_front(&interface::mode_update, this)),
                     [](sdbusplus::bus::match_t* match) { delete match; } },
      logger_(log_key) {}

void interface::set(tfc::operation::mode_e) const {}

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
      } catch (std::exception const& exc) {
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
