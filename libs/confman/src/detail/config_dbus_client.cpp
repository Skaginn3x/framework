#include <utility>

#include <tfc/confman/detail/config_dbus_client.hpp>
#include <tfc/dbus/exception.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/dbus/sdbusplus_meta.hpp>
#include <tfc/dbus/string_maker.hpp>
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

namespace dbus {
static constexpr std::string_view path{ tfc::dbus::const_dbus_path<config_path> };
}

config_dbus_client::config_dbus_client(boost::asio::io_context&) {}

config_dbus_client::config_dbus_client(dbus_connection_t conn,
                                       std::string_view key,
                                       value_call_t&& value_call,
                                       schema_call_t&& schema_call,
                                       change_call_t&& change_call)
    : interface_path_{ dbus::path }, interface_name_{ tfc::dbus::make_dbus_name(
                                         fmt::format("config.{}.{}.{}", base::get_exe_name(), base::get_proc_name(), key)) },
      value_call_{ std::move(value_call) }, schema_call_{ std::move(schema_call) }, change_call_{ std::move(change_call) },
      dbus_connection_{ std::move(conn) }, dbus_interface_{
        std::make_unique<sdbusplus::asio::dbus_interface>(dbus_connection_, interface_path_.string(), interface_name_)
      } {}
config_dbus_client::config_dbus_client(asio::io_context& ctx,
                                       std::string_view key,
                                       value_call_t&& value_call,
                                       schema_call_t&& schema_call,
                                       change_call_t&& change_call)
    : config_dbus_client(std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system()),
                         key,
                         std::move(value_call),
                         std::move(schema_call),
                         std::move(change_call)) {
  dbus_connection_->request_name(interface_name_.c_str());
}

config_dbus_client::config_dbus_client(interface_t intf,
                                       std::string_view key,
                                       value_call_t&& value_call,
                                       schema_call_t&& schema_call,
                                       change_call_t&& change_call)

    : property_name_{ key }, value_call_{ std::move(value_call) }, schema_call_{ std::move(schema_call) },
      change_call_{ std::move(change_call) }, dbus_interface_{ std::move(intf) } {}

void config_dbus_client::set(config_property&& prop) const {
  if (dbus_interface_) {
    dbus_interface_->set_property(std::string{ dbus::property_name }, prop);
  }
}

void config_dbus_client::initialize() {
  if (dbus_interface_) {
    dbus_interface_->register_property_rw<tfc::confman::detail::config_property>(
        property_name_, sdbusplus::vtable::property_::emits_change,
        [this]([[maybe_unused]] config_property const& req, [[maybe_unused]] config_property& old) -> int {  // setter
          if (req == old) {
            return 1;
          }
          auto err{ this->change_call_(req.value) };
          if (err) {
            throw tfc::dbus::exception::runtime{ fmt::format("Unable to save value: '{}', what: '{}'", req.value,
                                                             err.message()) };
          }
          old = req;  // this will populate some things for sdbusplus
          return 1;
        },
        [this]([[maybe_unused]] config_property const& value) -> config_property {  // getter
          return { .value = this->value_call_(), .schema = this->schema_call_() };
        });

    // If the provided interface is created by this class we initialize it here,
    // otherwise we assume it is taken care of elsewhere
    if (!interface_name_.empty()) {
      dbus_interface_->initialize();
    }
  }
}

auto config_dbus_client::get_io_context() const noexcept -> asio::io_context& {
  return dbus_interface_->connection()->get_io_context();
}

}  // namespace tfc::confman::detail
