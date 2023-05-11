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

static std::string replace_all(std::string const& input, std::string_view what, std::string_view with) {
  std::size_t count{};
  std::string copy{ input };
  for (std::string::size_type pos{}; std::string::npos != (pos = copy.find(what.data(), pos, what.length()));
       pos += with.length(), ++count) {
    copy.replace(pos, what.length(), with.data(), with.length());
  }
  return copy;
}

// clang-format off
// example get from cli
// busctl --verbose --user get-property com.skaginn3x.etc.tfc.confman_test.def.bar /com/skaginn3x/etc/tfc/confman_test/def/bar com.skaginn3x.etc.tfc.confman_test.def.bar config
// busctl --user introspect com.skaginn3x.etc.tfc.confman_test.def.bar /com/skaginn3x/etc/tfc/confman_test/def/bar
// clang-format on

config_dbus_client::config_dbus_client(boost::asio::io_context& ctx,
                                       std::string_view key,
                                       value_call_t&& value_call,
                                       schema_call_t&& schema_call,
                                       [[maybe_unused]] change_call_t&& change_call)
    : interface_path_{ std::filesystem::path{ tfc::dbus::make_dbus_path("") } /
                       base::make_config_file_name(key, "").string().substr(1) },
      interface_name_{ replace_all(interface_path_.string().substr(1), "/", ".") },
      dbus_connection_{ std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system()) },
      dbus_interface_{ std::make_unique<sdbusplus::asio::dbus_interface>(dbus_connection_, interface_path_.string(), interface_name_)
      } {
  dbus_interface_->register_property_rw<tfc::confman::detail::config_property>(
      std::string{ dbus::property_name.data(), dbus::property_name.size() }, sdbusplus::vtable::property_::emits_change,
      [change_call]([[maybe_unused]] config_property const& req, [[maybe_unused]] config_property& old) -> int {  // setter
        auto err{ change_call(req.value) };
        if (err) {
          throw tfc::dbus::exception::runtime{ fmt::format("Unable to save value: '{}', what: '{}'", req.value, err.message()) };
        }
        return 1;
      },
      [value_call, schema_call]([[maybe_unused]] config_property const& value) -> config_property {  // getter
        return { .value = value_call(), .schema = schema_call() };
      });

  dbus_connection_->request_name(interface_name_.c_str());

  dbus_interface_->initialize();
}

}  // namespace tfc::confman::detail
