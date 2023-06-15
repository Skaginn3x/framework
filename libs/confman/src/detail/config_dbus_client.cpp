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

static auto replace_all(std::string_view input, char what, char with) -> std::string {
  std::string copy{ input };
  std::replace(copy.begin(), copy.end(), what, with);  // replace all 'x' to 'y'
  return copy;
}

// clang-format off
// example get from cli
// busctl --verbose --user get-property com.skaginn3x.etc.tfc.confman_test.def.bar /com/skaginn3x/etc/tfc/confman_test/def/bar com.skaginn3x.etc.tfc.confman_test.def.bar config
// busctl --user introspect com.skaginn3x.etc.tfc.confman_test.def.bar /com/skaginn3x/etc/tfc/confman_test/def/bar
// clang-format on

config_dbus_client::config_dbus_client(boost::asio::io_context&) {}

config_dbus_client::config_dbus_client(boost::asio::io_context& ctx,
                                       std::string_view key,
                                       value_call_t&& value_call,
                                       schema_call_t&& schema_call,
                                       change_call_t&& change_call)
    : interface_path_{ std::filesystem::path{ tfc::dbus::make_dbus_path("config") } /
                       replace_all(base::make_config_file_name(key, "").string().substr(1),
                                   '-',
                                   '_') },  // dbus does not support dash in names
      interface_name_{ replace_all(interface_path_.string().substr(1), '/', '.') },
      value_call_{ std::forward<value_call_t>(value_call) }, schema_call_{ std::forward<schema_call_t>(schema_call) },
      change_call_{ std::forward<change_call_t>(change_call) },
      dbus_connection_{ std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system()) },
      dbus_interface_{
        std::make_unique<sdbusplus::asio::dbus_interface>(dbus_connection_, interface_path_.string(), interface_name_)
      } {
  dbus_interface_->register_property_rw<tfc::confman::detail::config_property>(
      std::string{ dbus::property_name.data(), dbus::property_name.size() }, sdbusplus::vtable::property_::emits_change,
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

  dbus_connection_->request_name(interface_name_.c_str());

  dbus_interface_->initialize();
}

void config_dbus_client::set(config_property&& prop) const {
  if (dbus_interface_) {
    dbus_interface_->set_property(std::string{ dbus::property_name.data(), dbus::property_name.size() }, prop);
  }
}

}  // namespace tfc::confman::detail
