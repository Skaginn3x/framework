#pragma once

#include <concepts>
#include <string>
#include <tuple>

#include <magic_enum.hpp>
#include <sdbusplus/message/append.hpp>
#include <sdbusplus/message/read.hpp>
#include <sdbusplus/message/types.hpp>

#include <tfc/stx/concepts.hpp>

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

template <>
struct type_id<long long> : tuple_type_id<SD_BUS_TYPE_INT64> {};

template <>
struct type_id<unsigned long long> : tuple_type_id<SD_BUS_TYPE_UINT64> {};

template <concepts::dbus_reflectable struct_t>
struct type_id<struct_t, void> {
  static constexpr auto value{ type_id<decltype(struct_t::dbus_reflection(struct_t{})), void>::value };
};

template <mp_units::Quantity quantity_t>
struct type_id<quantity_t, void> {
  static constexpr auto value{ type_id<typename quantity_t::rep, void>::value };
};

template <typename value_t, typename error_t>
struct type_id<std::expected<value_t, error_t>> {
  static constexpr auto value{ type_id<std::variant<value_t, error_t>>::value };
};
}  // namespace sdbusplus::message::types::details

namespace test {
enum struct foo : std::uint8_t { bar, baz };

static_assert(sdbusplus::message::types::details::type_id<
                  std::expected<mp_units::quantity<mp_units::si::milli<mp_units::si::gram>, std::uint64_t>, foo>>::value ==
              std::make_tuple('v'));
}  // namespace test

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

template <concepts::dbus_reflectable struct_t>
struct append_single<struct_t, void> {
  static void op(auto* interface, auto* sd_bus_msg, auto&& item) {
    append_single<decltype(struct_t::dbus_reflection(struct_t{})), void>::op(
        interface, sd_bus_msg, struct_t::dbus_reflection(std::forward<decltype(item)>(item)));
  }
};

template <mp_units::Quantity quantity_t>
struct append_single<quantity_t> {
  static void op(auto* interface, auto* sd_bus_msg, quantity_t& item) {
    using value_t = typename quantity_t::rep;
    append_single<value_t>::op(interface, sd_bus_msg, item.numerical_value_ref_in(quantity_t::unit));
  }
};

template <typename value_t, typename error_t>
struct append_single<std::expected<value_t, error_t>> {
  static void op(auto* interface, auto* sd_bus_msg, auto&& item) {
    std::variant<value_t, error_t> substitute{};
    if constexpr (std::is_rvalue_reference_v<decltype(item)>) {
      if (item.has_value()) {
        substitute.template emplace<value_t>(std::move(item.value()));
      } else {
        substitute.template emplace<error_t>(std::move(item.error()));
      }
    } else {
      if (item.has_value()) {
        substitute.template emplace<value_t>(item.value());
      } else {
        substitute.template emplace<error_t>(item.error());
      }
    }
    append_single<std::variant<value_t, error_t>>::op(interface, sd_bus_msg, substitute);
  }
};

template <concepts::dbus_reflectable struct_t>
struct read_single<struct_t> {
  static void op(auto* interface, auto* sd_bus_msg, auto& return_value) {
    using return_value_tuple_t = decltype(struct_t::dbus_reflection(struct_t{}));
    return_value_tuple_t return_value_tuple;
    read_single<return_value_tuple_t>::op(interface, sd_bus_msg, return_value_tuple);
    return_value = std::make_from_tuple<struct_t>(return_value_tuple);
  }
};

template <mp_units::Quantity quantity_t>
struct read_single<quantity_t> {
  static void op(auto* interface, auto* sd_bus_msg, mp_units::Quantity auto& return_value) {
    typename quantity_t::rep substitute{};
    read_single<typename quantity_t::rep>::op(interface, sd_bus_msg, substitute);
    return_value = substitute * quantity_t::reference;
  }
};

template <typename value_t, typename error_t>
struct read_single<std::expected<value_t, error_t>> {
  static void op(auto* interface, auto* sd_bus_msg, auto& return_value) {
    std::variant<value_t, error_t> substitute{};
    read_single<std::variant<value_t, error_t>>::op(interface, sd_bus_msg, substitute);
    std::visit(
        [&return_value]<typename item_t>(item_t&& itm) {
          if constexpr (std::same_as<std::remove_cvref_t<item_t>, value_t>) {
            return_value.emplace(std::move(itm));
          } else {
            return_value = std::unexpected{ std::move(itm) };
          }
        },
        substitute);
  }
};

}  // namespace sdbusplus::message::details
