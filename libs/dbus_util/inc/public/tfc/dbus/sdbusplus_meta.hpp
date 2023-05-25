#pragma once

#include <concepts>
#include <string>
#include <tuple>

#include <magic_enum.hpp>

#include <tfc/stx/concepts.hpp>

#define EXPORT __attribute__((visibility("default")))

namespace sdbusplus::concepts {

template <typename struct_t>
concept dbus_reflectable = requires {
                             struct_t::dbus_reflection;
                             requires std::invocable<decltype(struct_t::dbus_reflection), struct_t&&>;
                           };

}  // namespace sdbusplus::concepts

namespace sdbusplus::message::types::details {

template <typename value_t, typename enable_t>
struct type_id;

template <concepts::dbus_reflectable struct_t>
struct type_id<struct_t, void> {
  static constexpr auto value{ type_id<decltype(struct_t::dbus_reflection(struct_t{})), void>::value };
};

}  // namespace sdbusplus::message::types::details

namespace sdbusplus::message::details {

template <typename value_t>
struct convert_from_string;

template <typename value_t>
struct convert_to_string;

template <tfc::stx::is_enum enum_t>
struct convert_from_string<enum_t> {
  static auto op(const std::string& mode_str) noexcept -> std::optional<enum_t> {
    // This is really iffy magic_enum has flaws and making general use case for all enums might result in difficulties later
    return magic_enum::enum_cast<enum_t>(mode_str);
  }
};

template <tfc::stx::is_enum enum_t>
struct convert_to_string<enum_t> {
  static auto op(enum_t mode) -> std::string {
    // This is really iffy magic_enum has flaws and making general use case for all enums might result in difficulties later
    return std::string{ magic_enum::enum_name(mode) };
  }
};

template <typename struct_t, typename enable_t>
struct append_single;

template <concepts::dbus_reflectable struct_t>
struct append_single<struct_t, void> {
  static void op(auto* interface, auto* sd_bus_msg, auto&& item) {
    append_single<decltype(struct_t::dbus_reflection(struct_t{})), void>::op(
        interface, sd_bus_msg, struct_t::dbus_reflection(std::forward<decltype(item)>(item)));
  }
};

template <typename struct_t, typename enable_t>
struct read_single;

template <concepts::dbus_reflectable struct_t>
struct read_single<struct_t, void> {
  static void op(auto* interface, auto* sd_bus_msg, auto& return_value) {
    using return_value_tuple_t = decltype(struct_t::dbus_reflection(struct_t{}));
    return_value_tuple_t return_value_tuple;
    read_single<return_value_tuple_t, void>::op(interface, sd_bus_msg, return_value_tuple);
    return_value = std::make_from_tuple<struct_t>(return_value_tuple);
  }
};

}  // namespace sdbusplus::message::details
