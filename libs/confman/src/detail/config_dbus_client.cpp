#include <cassert>
#include <utility>

#include <tfc/confman/detail/config_dbus_client.hpp>
#include <tfc/dbus/exception.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/dbus/sdbusplus_meta.hpp>
#include <tfc/progbase.hpp>

#include <fmt/format.h>
#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

namespace tfc::confman::detail {

// clang-format off
// example get from cli
// busctl --verbose --system get-property com.skaginn3x.config.operation_mode.def.state_machine /com/skaginn3x/etc/tfc/config com.skaginn3x.config.operation_mode.def.state_machine config
// busctl --system introspect com.skaginn3x.config.operation_mode.def.state_machine /com/skaginn3x/etc/tfc/config
// clang-format on

config_dbus_client::config_dbus_client(std::shared_ptr<sdbusplus::asio::connection> conn) : dbus_connection_{ conn } {}

config_dbus_client::config_dbus_client(dbus_connection_t conn,
                                       std::string_view key,
                                       value_call_t&& value_call,
                                       schema_call_t&& schema_call,
                                       change_call_t&& change_call)
    : interface_path_{ tfc::dbus::make_dbus_path(key) }, value_call_{ std::move(value_call) },
      schema_call_{ std::move(schema_call) }, change_call_{ std::move(change_call) }, dbus_connection_{ std::move(conn) },
      dbus_interface_{
        std::make_unique<sdbusplus::asio::dbus_interface>(dbus_connection_, interface_path_.string(), interface_name_)
      } {}

void config_dbus_client::set(std::string&& prop) const {
  if (dbus_interface_) {
    dbus_interface_->set_property(value_property_name_, prop);
  }
}

void config_dbus_client::initialize() {
  if (dbus_interface_) {
    dbus_interface_->register_property_rw<std::string>(
        value_property_name_, sdbusplus::vtable::property_::emits_change,
        [this]([[maybe_unused]] std::string const& req, [[maybe_unused]] std::string& old) -> int {  // setter
          if (req == old) {
            return 1;
          }
          auto err{ this->change_call_(req) };
          if (err) {
            throw tfc::dbus::exception::runtime{ fmt::format("Unable to save value: '{}', what: '{}'", req, err.message()) };
          }
          old = req;  // this will populate some things for sdbusplus
          return 1;
        },
        [this]([[maybe_unused]] std::string const& value) -> std::string {  // getter
          return this->value_call_();
        });
    dbus_interface_->register_property_r<std::string>(
        schema_property_name_, sdbusplus::vtable::property_::emits_change,
        [this]([[maybe_unused]] std::string const& value) -> std::string {  // getter
          return this->schema_call_();
        });

    // If the provided interface is created by this class we initialize it here,
    // otherwise we assume it is taken care of elsewhere
    if (!interface_name_.empty()) {
      dbus_interface_->initialize();
    }
  }
}

auto config_dbus_client::get_io_context() const noexcept -> asio::io_context& {
  assert((dbus_connection_ || dbus_interface_) && "Invalid state");
  if (dbus_connection_) {
    return dbus_connection_->get_io_context();
  }
  return dbus_interface_->connection()->get_io_context();
}

}  // namespace tfc::confman::detail
