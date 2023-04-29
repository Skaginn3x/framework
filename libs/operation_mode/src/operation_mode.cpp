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

// template <tfc::stx::is_enum enum_t>
// struct type_id<enum_t> : type_id<std::underlying_type_t<enum_t>> {};

template <>
struct type_id<tfc::operation::update_message> {
  static constexpr auto value = std::tuple_cat(tuple_type_id_v<SD_BUS_TYPE_STRUCT_BEGIN>,
                                               type_id_v<tfc::operation::mode_e>,
                                               type_id_v<tfc::operation::mode_e>,  // todo generate type list
                                               tuple_type_id_v<SD_BUS_TYPE_STRUCT_END>);
};

struct foo {
  int a{};
  std::string b{};
  struct glaze {
    static constexpr auto value{ &foo::a };
    static constexpr std::string_view name{ "foo" };
  };
};

// glz object er bara tuplet::tuple

using std::string_view_literals::operator""sv;
static constexpr auto sd_bus_type_byte{ "y"sv };
static constexpr auto sd_bus_type_boolean{ "b"sv };
static constexpr auto sd_bus_type_int16{ "n"sv };
static constexpr auto sd_bus_type_uint16{ "q"sv };
static constexpr auto sd_bus_type_int32{ "i"sv };
static constexpr auto sd_bus_type_uint32{ "u"sv };
static constexpr auto sd_bus_type_int64{ "x"sv };
static constexpr auto sd_bus_type_uint64{ "t"sv };
static constexpr auto sd_bus_type_double{ "d"sv };
static constexpr auto sd_bus_type_string{ "s"sv };
static constexpr auto sd_bus_type_object_path{ "o"sv };
static constexpr auto sd_bus_type_signature{ "g"sv };
static constexpr auto sd_bus_type_unix_fd{ "h"sv };
static constexpr auto sd_bus_type_array{ "a"sv };
static constexpr auto sd_bus_type_variant{ "v"sv };
static constexpr auto sd_bus_type_struct{ "r"sv }; /* not actually used in signatures */
static constexpr auto sd_bus_type_struct_begin{ "("sv };
static constexpr auto sd_bus_type_struct_end{ ")"sv };
static constexpr auto sd_bus_type_dict_entry{ "e"sv }; /* not actually used in signatures */
static constexpr auto sd_bus_type_dict_entry_begin{ "{"sv };
static constexpr auto sd_bus_type_dict_entry_end{ "}"sv };

template <typename struct_t>
concept dbus_type_id_t = requires { struct_t::value; std::same_as<decltype(struct_t::value), std::string_view> };

template <typename some_t>
struct dbus_type_id;

template <typename value_t>
inline constexpr auto dbus_type_id_v = dbus_type_id<value_t>::value;

namespace detail {
template <typename char_t>
struct basic_object_path : public std::basic_string<char_t> {
  using std::basic_string<char_t>::basic_string;
};

template <typename char_t>
struct basic_signature : public std::basic_string<char_t> {
  using std::basic_string<char_t>::basic_string;
};
}
using object_path = detail::basic_object_path<char>;
using signature = detail::basic_signature<char>;
// todo unix_fd

// clang-format off
template <>
struct dbus_type_id<bool> { static constexpr auto value{ sd_bus_type_boolean }; };
template <>
struct dbus_type_id<std::uint8_t> { static constexpr auto value{ sd_bus_type_byte }; };
template <>
struct dbus_type_id<std::int8_t> : std::false_type {  };
template <>
struct dbus_type_id<std::uint16_t> { static constexpr auto value{ sd_bus_type_uint16 }; };
template <>
struct dbus_type_id<std::int16_t> { static constexpr auto value{ sd_bus_type_int16 }; };
template <>
struct dbus_type_id<std::uint32_t> { static constexpr auto value{ sd_bus_type_uint32 }; };
template <>
struct dbus_type_id<std::int32_t> { static constexpr auto value{ sd_bus_type_int32 }; };
template <>
struct dbus_type_id<std::uint64_t> { static constexpr auto value{ sd_bus_type_uint64 }; };
template <>
struct dbus_type_id<std::int64_t> { static constexpr auto value{ sd_bus_type_int64 }; };
template <>
struct dbus_type_id<double> { static constexpr auto value{ sd_bus_type_double }; };
template <>
struct dbus_type_id<std::string> { static constexpr auto value{ sd_bus_type_string }; };
template <>
struct dbus_type_id<std::string_view> { static constexpr auto value{ sd_bus_type_string }; };
template <>
struct dbus_type_id<const char*> { static constexpr auto value{ sd_bus_type_string }; };
template <>
struct dbus_type_id<char*> { static constexpr auto value{ sd_bus_type_string }; };
template <std::size_t size>
struct dbus_type_id<char[size]> { static constexpr auto value{ sd_bus_type_string }; };
template <>
struct dbus_type_id<object_path> { static constexpr auto value{ sd_bus_type_object_path }; };
template <>
struct dbus_type_id<signature> { static constexpr auto value{ sd_bus_type_signature }; };
// todo unix_fd
// clang-format on

static_assert(dbus_type_id_v<bool> == "b"sv);
static_assert(dbus_type_id_v<uint8_t> == "y"sv);
//static_assert(std::is_convertible_v<decltype(dbus_type_id_v<int8_t>), std::false_type>);
static_assert(dbus_type_id_v<int16_t> == "n"sv);
static_assert(dbus_type_id_v<uint16_t> == "q"sv);
static_assert(dbus_type_id_v<int32_t> == "i"sv);
static_assert(dbus_type_id_v<uint32_t> == "u"sv);
static_assert(dbus_type_id_v<int64_t> == "x"sv);
static_assert(dbus_type_id_v<uint64_t> == "t"sv);
static_assert(dbus_type_id_v<double> == "d"sv);
static_assert(dbus_type_id_v<std::string> == "s"sv);
static_assert(dbus_type_id_v<std::string_view> == "s"sv);
static_assert(dbus_type_id_v<char*> == "s"sv);
static_assert(dbus_type_id_v<char const*> == "s"sv);
static_assert(dbus_type_id_v<char[40]> == "s"sv);
static_assert(dbus_type_id_v<object_path> == "o"sv);
static_assert(dbus_type_id_v<signature> == "g"sv);



template <typename struct_t>
concept glaze_transposable = requires { struct_t::glaze::value; };

template <glaze_transposable struct_t>
struct dbus_type_id<struct_t> {
  static constexpr auto value = std::tuple_cat(type_id_v<std::remove_pointer_t<decltype(struct_t::glaze::value)>>);
};

static_assert(type_id<foo>::value == std::make_tuple('i'));
//
// template <typename type>
// struct type_id<glz::meta<type>> {
//
//};

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
