#include <tfc/ipc/details/dbus_client_iface.hpp>

#include <glaze/glaze.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/property.hpp>
#include <sdbusplus/bus/match.hpp>

#include <tfc/dbus/match_rules.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/dbus/string_maker.hpp>
#include <tfc/ipc/details/dbus_structs_glaze_meta.hpp>
#include <tfc/ipc/glaze_meta.hpp>

namespace tfc::ipc_ruler {

ipc_manager_client::ipc_manager_client(asio::io_context& ctx)
    : ipc_manager_client(std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system())) {
  // If the owner of this client does not supply dbus connection we assume we are the sole owner of the connection
  connection_->request_name(tfc::dbus::make_dbus_process_name().c_str());
}

ipc_manager_client::ipc_manager_client(std::shared_ptr<sdbusplus::asio::connection> connection)
    : connection_match_rule_{ tfc::dbus::match::rules::make_match_rule<consts::ipc_ruler_service_name,
                                                                       consts::ipc_ruler_interface_name,
                                                                       consts::ipc_ruler_object_path,
                                                                       tfc::dbus::match::rules::type::signal>() },
      connection_{ std::move(connection) },
      connection_match_{ make_match(connection_match_rule_, std::bind_front(&ipc_manager_client::match_callback, this)) } {}

ipc_manager_client::ipc_manager_client(ipc_manager_client&& to_be_erased) noexcept {
  connection_ = std::move(to_be_erased.connection_);
  slot_callbacks_ = std::move(to_be_erased.slot_callbacks_);
  // It is pretty safe to construct new match here it mostly invokes C api where it does not explicitly throw
  // it could throw if we are out of memory, but then we are already screwed and the process will terminate.
  connection_match_ = make_match(connection_match_rule_, std::bind_front(&ipc_manager_client::match_callback, this));
}
auto ipc_manager_client::operator=(ipc_manager_client&& to_be_erased) noexcept -> ipc_manager_client& {
  connection_ = std::move(to_be_erased.connection_);
  slot_callbacks_ = std::move(to_be_erased.slot_callbacks_);
  // It is pretty safe to construct new match here it mostly invokes C api where it does not explicitly throw
  // it could throw if we are out of memory, but then we are already screwed and the process will terminate.
  connection_match_ = make_match(connection_match_rule_, std::bind_front(&ipc_manager_client::match_callback, this));
  return *this;
}
auto ipc_manager_client::register_signal(const std::string_view name,
                                         const std::string_view description,
                                         ipc::details::type_e type,
                                         std::function<void(std::error_code const&)>&& handler) -> void {
  connection_->async_method_call(std::move(handler), ipc_ruler_service_name_, ipc_ruler_object_path_,
                                 ipc_ruler_interface_name_, consts::register_signal.data(), name, description,
                                 static_cast<uint8_t>(type));
}
auto ipc_manager_client::register_slot(const std::string_view name,
                                       const std::string_view description,
                                       ipc::details::type_e type,
                                       std::function<void(std::error_code const&)>&& handler) -> void {
  connection_->async_method_call(std::move(handler), ipc_ruler_service_name_, ipc_ruler_object_path_,
                                 ipc_ruler_interface_name_, "RegisterSlot", name, description, static_cast<uint8_t>(type));
}
auto ipc_manager_client::signals(std::function<void(std::vector<signal> const&)>&& handler) -> void {
  sdbusplus::asio::getProperty<std::string>(
      *connection_, ipc_ruler_service_name_, ipc_ruler_object_path_, ipc_ruler_interface_name_,
      consts::signals_property.data(),
      [captured_handler = std::move(handler)](const boost::system::error_code& error, const std::string& response) {
        if (error) {
          return;
        }
        auto signals = glz::read_json<std::vector<signal>>(response);
        if (signals) {
          captured_handler(signals.value());
        }
      });
}
auto ipc_manager_client::slots(std::function<void(std::vector<slot> const&)>&& handler) -> void {
  sdbusplus::asio::getProperty<std::string>(
      *connection_, ipc_ruler_service_name_, ipc_ruler_object_path_, ipc_ruler_interface_name_,
      consts::slots_property.data(),
      [captured_handler = std::move(handler)](const boost::system::error_code& error, const std::string& response) {
        if (error) {
          return;
        }
        auto slots = glz::read_json<std::vector<slot>>(response);
        if (slots) {
          captured_handler(slots.value());
        }
      });
}
auto ipc_manager_client::connections(std::function<void(std::map<std::string, std::vector<std::string>> const&)>&& handler)
    -> void {
  sdbusplus::asio::getProperty<std::string>(
      *connection_, ipc_ruler_service_name_, ipc_ruler_object_path_, ipc_ruler_interface_name_,
      consts::connections_property.data(),
      [handl = std::move(handler)](const boost::system::error_code& error, const std::string& response) {
        if (error) {
          return;
        }
        auto slots = glz::read_json<std::map<std::string, std::vector<std::string>>>(response);
        if (slots) {
          handl(slots.value());
        }
      });
}
auto ipc_manager_client::connect(std::string_view slot_name,
                                 std::string_view signal_name,
                                 std::function<void(std::error_code const&)>&& handler) -> void {
  connection_->async_method_call(std::move(handler), ipc_ruler_service_name_, ipc_ruler_object_path_,
                                 ipc_ruler_interface_name_, consts::connect_method.data(), slot_name, signal_name);
}
auto ipc_manager_client::disconnect(std::string_view slot_name, std::function<void(std::error_code const&)>&& handler)
    -> void {
  connection_->async_method_call(std::move(handler), ipc_ruler_service_name_, ipc_ruler_object_path_,
                                 ipc_ruler_interface_name_, consts::disconnect_method.data(), slot_name);
}
auto ipc_manager_client::register_connection_change_callback(
    std::string_view slot_name,
    const std::function<void(const std::string_view)>& connection_change_callback) -> void {
  slot_callbacks_.emplace(std::string{ slot_name }, connection_change_callback);
}
auto ipc_manager_client::register_properties_change_callback(
    const std::function<void(sdbusplus::message_t&)>& match_change_callback)
    -> std::unique_ptr<sdbusplus::bus::match::match> {
  return make_match(sdbusplus::bus::match::rules::propertiesChanged(ipc_ruler_object_path_, ipc_ruler_interface_name_),
                    match_change_callback);
}
auto ipc_manager_client::make_match(const std::string& match_rule,
                                    const std::function<void(sdbusplus::message_t&)>& callback)
    -> std::unique_ptr<sdbusplus::bus::match::match> {
  return std::make_unique<sdbusplus::bus::match::match>(*connection_, match_rule, callback);
}
auto ipc_manager_client::match_callback(sdbusplus::message_t& msg) -> void {
  auto container = msg.unpack<std::tuple<std::string, std::string>>();
  std::string const slot_name = std::get<0>(container);
  std::string const signal_name = std::get<1>(container);
  auto iterator = slot_callbacks_.find(slot_name);
  if (iterator != slot_callbacks_.end()) {
    std::invoke(iterator->second, signal_name);
  }
}

}  // namespace tfc::ipc_ruler
