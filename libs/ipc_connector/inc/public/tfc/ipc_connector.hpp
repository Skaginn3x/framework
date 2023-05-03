#pragma once

#include <tfc/confman.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc/glaze_meta.hpp>
#include <tfc/ipc_connector/dbus_server_iface.hpp>

namespace tfc::ipc {

namespace asio = boost::asio;

template <typename type_desc>
class slot_configurable {
public:
  using value_t = slot_callback<type_desc>::value_t;
  static constexpr std::string_view type_name{ type_desc::type_name };
  static auto constexpr direction_v = slot_callback<type_desc>::direction_v;

  slot_configurable(asio::io_context& ctx, std::string_view name, auto&& callback)
      : slot_(slot_callback<type_desc>::create(ctx, name)), client_(ctx) {
    client_.register_connection_change_callback([this, callback](std::string slot_name, std::string signal_name) {
      if (slot_name == slot_->name_w_type()) {
        if (signal_name.empty()) {
          slot_->disconnect("");
        } else {
          slot_->init(signal_name, callback);
        }
      }
    });
    client_.register_slot(slot_->name_w_type(), type_desc::value_e, [](boost::system::error_code const& error_code) {
      if (error_code) {
        throw std::runtime_error("Error registering slot");
      }
    });
  }

  slot_configurable(slot_configurable&) noexcept = default;
  slot_configurable(slot_configurable&&) noexcept = default;
  auto operator=(slot_configurable&&) noexcept -> slot_configurable& = default;
  auto operator=(slot_configurable const&) noexcept -> slot_configurable& = default;

  [[nodiscard]] auto value() const noexcept { return slot_->get(); }  // todo: value() or get()

private:
  static constexpr std::string_view self_name{ slot_tag };
  std::shared_ptr<slot_callback<type_desc>> slot_;
  ipc_ruler::ipc_manager_client client_;
};

template <typename type_desc>
class signal_exposed {
public:
  using value_t = signal<type_desc>::value_t;
  static constexpr std::string_view type_name{ type_desc::type_name };
  static auto constexpr direction_v = signal<type_desc>::direction_v;

  signal_exposed(asio::io_context& ctx, std::string_view name)
      : signal_{ signal<type_desc>::create(ctx, name).value() }, client_(ctx) {
    client_.register_signal(signal_->name_w_type(), type_desc::value_e, [](boost::system::error_code const& error_code){
      if (error_code){
        throw std::runtime_error("Error registering slot");
      }
    });
  }
  auto send(value_t const& value) -> std::error_code { return signal_->send(value); }

  auto async_send(value_t const& value, std::invocable<std::error_code, std::size_t> auto&& callback) -> void {
    signal_->async_send(value, std::forward<decltype(callback)>(callback));
  }

private:
  static constexpr std::string_view self_name{ signal_tag };
  std::shared_ptr<signal<type_desc>> signal_;
  ipc_ruler::ipc_manager_client client_;
};

using bool_recv_conf_cb = slot_configurable<type_bool>;
using int_recv_conf_cb = slot_configurable<type_int>;
using uint_recv_conf_cb = slot_configurable<type_uint>;
using double_recv_conf_cb = slot_configurable<type_double>;
using string_recv_conf_cb = slot_configurable<type_string>;
using json_recv_conf_cb = slot_configurable<type_json>;

using bool_send_exposed = signal_exposed<type_bool>;
using int_send_exposed = signal_exposed<type_int>;
using uint_send_exposed = signal_exposed<type_uint>;
using double_send_exposed = signal_exposed<type_double>;
using string_send_exposed = signal_exposed<type_string>;
using json_send_exposed = signal_exposed<type_json>;

}  // namespace tfc::ipc
