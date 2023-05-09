#include <tfc/confman/detail/config_dbus_client.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/dbus/sdbusplus_meta.hpp>
#include <tfc/dbus/string_maker.hpp>
#include <tfc/progbase.hpp>
#include <tfc/stx/to_tuple.hpp>

#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

namespace tfc::confman::detail {

struct config_property {
  // THIS SHOULD BE string_view, sdbusplus does not support it as std::string_view is not convertible to char const*
  std::string value{};
  std::string schema{};
  static constexpr auto dbus_reflection{ [](auto&& self) {
    return tfc::stx::to_tuple(std::forward<decltype(self)>(self));
  } };
};

static auto operator==(config_property const& lhs, config_property const& rhs) noexcept -> bool {
  return lhs.value == rhs.value && lhs.schema == rhs.schema;
}

static std::string replace_all(std::string const& input, std::string_view what, std::string_view with) {
  std::size_t count{};
  std::string copy{ input };
  for (std::string::size_type pos{}; std::string::npos != (pos = copy.find(what.data(), pos, what.length()));
       pos += with.length(), ++count) {
    copy.replace(pos, what.length(), with.data(), with.length());
  }
  return copy;
}
static std::string remove_first_char(std::string const& input) {
  std::string copy{ input };
  copy.erase(std::begin(copy));
  return copy;
}

config_dbus_client::config_dbus_client(boost::asio::io_context& ctx, std::string_view key)
    : interface_path_{ dbus::make_dbus_path(base::make_config_file_name(key, "").string()) },
      interface_name_{ remove_first_char(replace_all(replace_all(interface_path_.string(), "/", "."), "..", ".")) },
      dbus_connection_{ std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system()) },
      dbus_object_server_{ std::make_unique<sdbusplus::asio::object_server>(dbus_connection_) },
      dbus_interface_{ dbus_object_server_->add_unique_interface(interface_path_.string(), interface_name_) } {
  //  int nop_set_value(const PropertyType& req, PropertyType& old)
  //  PropertyType nop_get_value(const PropertyType& value)

  dbus_interface_->register_property_rw<tfc::confman::detail::config_property>(
      std::string{ key.data(), key.size() }, sdbusplus::vtable::property_::emits_change,
      []([[maybe_unused]] config_property const& req,[[maybe_unused]] config_property& old) -> int {
        return 1;
      },
      [](config_property const& value) -> config_property { return value; });

  dbus_connection_->request_name(interface_name_.c_str());

  dbus_interface_->initialize();
}

}  // namespace tfc::confman::detail
