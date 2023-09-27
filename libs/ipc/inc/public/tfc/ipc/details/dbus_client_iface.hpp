#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include <tfc/dbus/sdbusplus_fwd.hpp>
#include <tfc/ipc/details/dbus_constants.hpp>
#include <tfc/ipc/details/dbus_structs.hpp>
#include <tfc/ipc/enums.hpp>
#include <tfc/utils/asio_fwd.hpp>

namespace tfc::ipc_ruler {

namespace asio = boost::asio;

class ipc_manager_client {
public:
  explicit ipc_manager_client(asio::io_context& ctx);

  /// \param connection non null pointer to a sdbusplus connection
  explicit ipc_manager_client(std::shared_ptr<sdbusplus::asio::connection> connection);
  // copy constructors should not be used
  ipc_manager_client(ipc_manager_client const&) = delete;
  auto operator=(ipc_manager_client const&) -> ipc_manager_client& = delete;
  ipc_manager_client(ipc_manager_client&& to_be_erased) noexcept;
  auto operator=(ipc_manager_client&& to_be_erased) noexcept -> ipc_manager_client&;

  /**
   * Register a signal with the ipc_manager service running on dbus
   * @param name the name of the signal to be registered
   * @param type  the type enum of the signal to be registered
   * @param handler  the error handling callback function
   */
  auto register_signal(std::string_view name,
                       std::string_view description,
                       ipc::details::type_e type,
                       std::function<void(std::error_code const&)>&& handler) -> void;

  /**
   * Register a slot with the ipc_manager service running on dbus
   * @param name the name of the slot to be registered
   * @param type  the type enum of the slot to be registered
   * @param handler  the error handling callback function
   */
  auto register_slot(std::string_view name,
                     std::string_view description,
                     ipc::details::type_e type,
                     std::function<void(std::error_code const&)>&& handler) -> void;

  /**
   * Async function to get the signals property from the ipc manager
   * This fetches the signals over dbus and then calls the provided callback with
   * a vector of signals
   * @param handler a function like object that is called back with a vector of signals
   */
  auto signals(std::function<void(std::vector<signal> const&)>&& handler) -> void;

  /**
   * Async function to get the slots property from the ipc manager
   * This fetches the slots over dbus and then calls the provided callback with
   * a vector of slots
   * @param handler a function like object that is called back with a vector of slots
   */
  auto slots(std::function<void(std::vector<slot> const&)>&& handler) -> void;

  /**
   * Async function to get the connections from the ipc manager.
   * @param handler  a function like object that is called with a map og strings and a vector of strings
   */
  auto connections(std::function<void(std::map<std::string, std::vector<std::string>> const&)>&& handler) -> void;

  /**
   * Send a request over dbus to connect a slot to a signal.
   * On error handler will be called back with a non empty std::error_code&
   * @param slot_name the name of the slot to be connected
   * @param signal_name the name of the signal to be connected
   * @param handler a method that receives an error in case there is one
   * @note If the slot is already connected to a signal, it will be disconnected and connected to the new one
   */
  auto connect(std::string_view slot_name,
               std::string_view signal_name,
               std::function<void(std::error_code const&)>&& handler) -> void;

  /**
   * Send a request over dbus to disconnect a slot from its signal
   * @param slot_name the name of the slot to be disconnected
   * @param handler a method that receives an error in case there is one
   */
  auto disconnect(std::string_view slot_name, std::function<void(std::error_code const&)>&& handler) -> void;

  /**
   * Register a callback function that gets "pinged" each time there is a connection change
   * @param connection_change_callback a function like object on each connection change it gets called with the slot and
   * signal that changed.
   * @note There can only be a single callback registered per slot_name, if you register a new one. The old callback will be
   * overwritten.
   */
  auto register_connection_change_callback(std::string_view slot_name,
                                           const std::function<void(std::string_view const)>& connection_change_callback)
      -> void;

  /**
   * Register a callback function that gets "pinged" each time there is a change in the properties of the ipc manager
   * @param match_change_callback a function like object on each property change that gets called with the dbus message
   * @return a unique pointer to the match object, this is needed to keep the match object alive
   * @note The match object is kept alive by the unique pointer, if the unique pointer is destroyed the match object will be
   * destroyed.
   */
  auto register_properties_change_callback(std::function<void(sdbusplus::message_t&)> const& match_change_callback)
      -> std::unique_ptr<sdbusplus::bus::match::match>;

private:
  auto make_match(const std::string& match_rule, std::function<void(sdbusplus::message_t&)> const& callback)
      -> std::unique_ptr<sdbusplus::bus::match::match>;
  auto match_callback(sdbusplus::message_t& msg) -> void;
  const std::string ipc_ruler_service_name_{ consts::ipc_ruler_service_name };
  const std::string ipc_ruler_interface_name_{ consts::ipc_ruler_interface_name };
  const std::string ipc_ruler_object_path_{ consts::ipc_ruler_object_path };

  std::string connection_match_rule_{};
  std::shared_ptr<sdbusplus::asio::connection> connection_;
  std::unique_ptr<sdbusplus::bus::match::match, std::function<void(sdbusplus::bus::match::match*)>> connection_match_;
  std::unordered_map<std::string, std::function<void(std::string_view const)>> slot_callbacks_;
};

}  // namespace tfc::ipc_ruler
