#pragma once

#include <system_error>

#include <fmt/format.h>

#include <tfc/dbus/sdbusplus_fwd.hpp>
#include <tfc/ipc/details/dbus_client_iface.hpp>
#include <tfc/ipc/details/dbus_ipc.hpp>
#include <tfc/ipc/details/filter.hpp>
#include <tfc/ipc/details/impl.hpp>
#include <tfc/ipc/details/type_description.hpp>
#include <tfc/ipc/enums.hpp>
#include <tfc/ipc/glaze_meta.hpp>
#include <tfc/stx/concepts.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace tfc::ipc {

namespace asio = boost::asio;

/// \brief make manager client for the types(signal, slot) below
[[maybe_unused]] static auto make_manager_client(asio::io_context& ctx) {
  return ipc_ruler::ipc_manager_client{ ctx };
}

/**
 * @brief
 * This is the receiving end for tfc's ipc communications,
 * it listens to connection changes from ipc-ruler in
 * addition with implementing the ipc connection.
 * @tparam type_desc The type description for the slot.
 */
template <typename type_desc, typename manager_client_type = ipc_ruler::ipc_manager_client&>
class slot {
public:
  using value_t = typename details::slot_callback<type_desc>::value_t;
  static constexpr std::string_view type_name{ type_desc::type_name };
  static auto constexpr direction_v = details::slot_callback<type_desc>::direction_v;

  /**
   * C'tor for a tfc IPC slot.
   * @param ctx Execution context
   * @param client manager_client_type a reference to a manager client
   * @param name The slot name
   * @param callback Channel for value updates from the corresponding signal.
   */
  slot(asio::io_context& ctx,
       manager_client_type client,
       std::string_view name,
       std::string_view description,
       stx::invocable<value_t> auto&& callback)
    requires std::is_lvalue_reference_v<manager_client_type>
      : slot_{ details::slot_callback<type_desc>::create(ctx, name) }, dbus_slot_{ client.connection(), slot_->type_name() },
        client_{ client }, filters_{ client.connection(), slot_->type_name(),
                                     // store the callers callback in this lambda
                                     [this, callb = std::forward<decltype(callback)>(callback)](value_t const& new_value) {
                                       callb(new_value);
                                       dbus_slot_.emit_value(new_value);
                                     } } {
    client_init(description);
  }

  slot(asio::io_context& ctx,
       std::shared_ptr<sdbusplus::asio::connection> connection,
       std::string_view name,
       std::string_view description,
       tfc::stx::invocable<value_t> auto&& callback)
    requires(!std::is_lvalue_reference_v<manager_client_type>)
      : slot_{ details::slot_callback<type_desc>::create(ctx, name) }, dbus_slot_{ connection, slot_->type_name() },
        client_{ connection },
        filters_{ connection, slot_->type_name(),
                  // store the callers callback in this lambda
                  [this, callb = std::forward<decltype(callback)>(callback)](value_t const& new_value) {
                    callb(new_value);
                    dbus_slot_.emit_value(new_value);
                  } } {
    client_init(description);
  }

  slot(asio::io_context& ctx,
       manager_client_type client,
       std::string_view name,
       stx::invocable<value_t> auto&& callback)
      : slot(ctx, client, name, "", std::forward<decltype(callback)>(callback)) {}

  slot(slot&) = delete;

  slot(slot&&) noexcept = delete;

  auto operator=(slot&&) noexcept -> slot& = delete;

  auto operator=(slot const&) -> slot& = delete;

  [[nodiscard]] auto value() const noexcept -> std::optional<value_t> const& { return filters_.value(); }

  [[nodiscard]] auto unfiltered_value() const noexcept -> std::optional<value_t> const& { return slot_.value(); }

  [[nodiscard]] auto name() const noexcept -> std::string_view { return slot_->name(); }

  [[nodiscard]] auto full_name() const noexcept -> std::string { return slot_->full_name(); }

  /**
   * @brief Get the connected signal name
   * Can be used to compare two connection of two slots or check whether the slot is connected in general.
   */
  [[nodiscard]] auto connection() const noexcept -> auto const& { return connected_signal_; }

private:
  void client_init(std::string_view description) {
    client_.register_connection_change_callback(full_name(), [this](std::string_view signal_name) {
      connected_signal_ = signal_name;
      slot_->connect(signal_name, filters_);
    });

    client_.register_slot_retry(full_name(), description, type_desc::value_e);

    dbus_slot_.on_set([this](value_t&& set_value) { this->filters_.set(std::move(set_value)); });

    dbus_slot_.initialize();
  }

  std::shared_ptr<details::slot_callback<type_desc>> slot_;
  details::dbus_ipc<value_t, details::ipc_type_e::slot> dbus_slot_;
  manager_client_type client_;
  filter::filters<value_t, std::function<void(value_t const&)>> filters_;
  std::optional<std::string> connected_signal_{ std::nullopt };
};

/**
 * an ipc component, registers its existence with an
 * ipc-ruler service.
 * @tparam type_desc The type of the signal
 */
template <typename type_desc, typename manager_client_type = ipc_ruler::ipc_manager_client&>
class signal {
public:
  using value_t = typename details::signal<type_desc>::value_t;
  static constexpr auto value_type{ type_desc::value_e };
  using dbus_signal_t = details::dbus_ipc<value_t, details::ipc_type_e::signal>;
  static constexpr std::string_view type_name{ type_desc::type_name };
  static auto constexpr direction_v = details::signal<type_desc>::direction_v;

  /**
   * Signal c'tor
   * @param ctx Execution context
   * @param name Signals name
   */
  signal(asio::io_context& ctx, manager_client_type client, std::string_view name, std::string_view description = "")
    requires std::is_lvalue_reference_v<manager_client_type>
      : client_{ client }, signal_{ make_impl_signal(ctx, name) },
        dbus_signal_{ client_.connection(), signal_->type_name() } {
    client_.register_signal_retry(signal_->full_name(), description, type_desc::value_e);
    dbus_signal_.initialize();
  }

  signal(asio::io_context& ctx,
         std::shared_ptr<sdbusplus::asio::connection> connection,
         std::string_view name,
         std::string_view description = "")
    requires(!std::is_lvalue_reference_v<manager_client_type>)
      : client_{ connection }, signal_{ make_impl_signal(ctx, name) },
        dbus_signal_{ client_.connection(), signal_->type_name() } {
    client_.register_signal_retry(signal_->full_name(), description, type_desc::value_e);
    dbus_signal_.initialize();
  }

  signal(signal&&) noexcept = default;
  auto operator=(signal&&) noexcept -> signal& = default;
  signal(signal const&) = delete;
  auto operator=(signal const&) -> signal& = delete;
  ~signal() noexcept = default;

  auto send(value_t const& value) -> std::error_code {
    auto err{ signal_->send(value) };
    if (!err) {
      dbus_signal_.emit_value(value);
    }
    return err;
  }

  template <asio::completion_token_for<void(std::error_code, std::size_t)> completion_token_t>
  auto async_send(value_t const& value, completion_token_t&& token) -> auto {
    dbus_signal_.emit_value(value);  // Todo: we should wrap the token and embed this into the completion_token handle
    return signal_->async_send(value, std::forward<completion_token_t>(token));
  }

  [[nodiscard]] auto name() const noexcept -> std::string_view { return signal_->name(); }

  [[nodiscard]] auto full_name() const noexcept -> std::string { return signal_->full_name(); }

  [[nodiscard]] auto value() const noexcept -> std::optional<value_t> const& { return signal_->value(); }

private:
  static auto make_impl_signal(auto&& ctx, auto&& name) {
    auto exp{ details::signal<type_desc>::create(ctx, name) };
    if (!exp.has_value()) {
      throw std::runtime_error{ fmt::format("Unable to bind to socket, reason: {}", exp.error().message()) };
    }
    return std::move(exp.value());
  }

  manager_client_type client_;
  std::shared_ptr<details::signal<type_desc>> signal_;
  dbus_signal_t dbus_signal_;
};

template <typename return_t, typename manager_client_t, template <typename, typename> typename ipc_base_t>
struct make_any;

using bool_slot = slot<details::type_bool>;
using int_slot = slot<details::type_int>;
using uint_slot = slot<details::type_uint>;
using double_slot = slot<details::type_double>;
using string_slot = slot<details::type_string>;
using json_slot = slot<details::type_json>;
using mass_slot = slot<details::type_mass>;
using length_slot = slot<details::type_length>;
using pressure_slot = slot<details::type_pressure>;
using temperature_slot = slot<details::type_temperature>;
using voltage_slot = slot<details::type_voltage>;
using current_slot = slot<details::type_current>;
using any_slot = std::variant<std::monostate,
                              bool_slot,
                              int_slot,
                              uint_slot,
                              double_slot,
                              string_slot,
                              json_slot,
                              mass_slot,
                              length_slot,
                              pressure_slot,
                              temperature_slot,
                              voltage_slot,
                              current_slot>;
/// \brief any_slot foo = make_any_slot(type_e::bool, ctx, client, "name", "description", [](bool new_state){});
using make_any_slot = make_any<any_slot, ipc_ruler::ipc_manager_client&, slot>;

using bool_signal = signal<details::type_bool>;
using int_signal = signal<details::type_int>;
using uint_signal = signal<details::type_uint>;
using double_signal = signal<details::type_double>;
using string_signal = signal<details::type_string>;
using json_signal = signal<details::type_json>;
using mass_signal = signal<details::type_mass>;
using length_signal = signal<details::type_length>;
using pressure_signal = signal<details::type_pressure>;
using temperature_signal = signal<details::type_temperature>;
using voltage_signal = signal<details::type_voltage>;
using current_signal = signal<details::type_current>;
using any_signal = std::variant<std::monostate,
                                bool_signal,
                                int_signal,
                                uint_signal,
                                double_signal,
                                string_signal,
                                json_signal,
                                mass_signal,
                                length_signal,
                                pressure_signal,
                                temperature_signal,
                                voltage_signal,
                                current_signal>;
/// \brief any_signal foo = make_any_signal::make(type_e::bool, ctx, client, "name", "description");
using make_any_signal = make_any<any_signal, ipc_ruler::ipc_manager_client&, signal>;

template <typename return_t, typename manager_client_t, template <typename, typename> typename ipc_base_t>
struct make_any {
  static_assert(std::is_lvalue_reference_v<manager_client_t>, "manager_client_t must be a reference");
  static auto make(details::type_e type, auto&&... args) -> return_t {
    switch (type) {
      using enum details::type_e;
      case _bool:
        return ipc_base_t<details::type_bool, manager_client_t>{ std::forward<decltype(args)>(args)... };
      case _int64_t:
        return ipc_base_t<details::type_int, manager_client_t>{ std::forward<decltype(args)>(args)... };
      case _uint64_t:
        return ipc_base_t<details::type_uint, manager_client_t>{ std::forward<decltype(args)>(args)... };
      case _double_t:
        return ipc_base_t<details::type_double, manager_client_t>{ std::forward<decltype(args)>(args)... };
      case _string:
        return ipc_base_t<details::type_string, manager_client_t>{ std::forward<decltype(args)>(args)... };
      case _json:
        return ipc_base_t<details::type_json, manager_client_t>{ std::forward<decltype(args)>(args)... };
      case _mass:
        return ipc_base_t<details::type_mass, manager_client_t>{ std::forward<decltype(args)>(args)... };
      case _length:
        return ipc_base_t<details::type_length, manager_client_t>{ std::forward<decltype(args)>(args)... };
      case _pressure:
        return ipc_base_t<details::type_pressure, manager_client_t>{ std::forward<decltype(args)>(args)... };
      case _temperature:
        return ipc_base_t<details::type_temperature, manager_client_t>{ std::forward<decltype(args)>(args)... };
      case _voltage:
        return ipc_base_t<details::type_voltage, manager_client_t>{ std::forward<decltype(args)>(args)... };
      case _current:
        return ipc_base_t<details::type_current, manager_client_t>{ std::forward<decltype(args)>(args)... };
      case unknown:
        return std::monostate{};
    }
    return {};
  }
};

}  // namespace tfc::ipc
