#include <chrono>
#include <cstdint>
#include <string>

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <mp-units/systems/si.h>
#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/progbase.hpp>
#include <tfc/stx/glaze_meta.hpp>
#include <tfc/utils/json_schema.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace asio = boost::asio;

struct option_1 {
  mp_units::quantity<mp_units::si::deci<mp_units::si::ampere>, uint16_t> amper{};
  struct glaze {
    using type = option_1;
    static constexpr auto value{ glz::object("amper", &type::amper, "amper description") };
    static constexpr std::string_view name{ "option_1" };
  };
  constexpr auto operator==(option_1 const& rhs) const noexcept -> bool = default;
};

struct option_2 {
  std::string a{};
  tfc::confman::observable<std::chrono::nanoseconds> sec{};
  struct glaze {
    using type = option_2;
    static constexpr auto value{ glz::object("a",
                                             &type::a,
                                             "A description",
                                             "sec",
                                             &type::sec,
                                             tfc::json::schema{ .description = "sec desc", .minimum = 30, .maximum = 40 }) };
    static constexpr std::string_view name{ "option_2" };
  };
  constexpr auto operator==(option_2 const& rhs) const noexcept -> bool = default;
};

struct with_variant {
  int a{};
  std::variant<std::monostate, option_1, option_2> variant{};
  struct glaze {
    using type = with_variant;
    static constexpr auto value{
      glz::object("a_int", &type::a, "A int description", "variant", &type::variant, "variant description")
    };
    static constexpr std::string_view name{ "with_variant" };
  };
  constexpr auto operator==(with_variant const& rhs) const noexcept -> bool = default;
};

int main(int argc, char** argv) {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};
  auto dbus{ std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system()) };

  tfc::confman::config<tfc::confman::observable<std::vector<with_variant>>> const config{ dbus, "key" };
  config->observe([](auto const& new_value, auto const& old_value) {
    fmt::print("new value:\n{}\n\n\nold value:\n{}\n", glz::write_json(new_value).value(),
               glz::write_json(old_value).value());
  });

  fmt::print("Schema is: {}\n", config.schema());
  fmt::print("Config is: {}\n", config.string().value());

  dbus->request_name(tfc::dbus::make_dbus_process_name().c_str());

  ctx.run();
  return 0;
}
