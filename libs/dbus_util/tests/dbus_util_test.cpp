#include <string_view>

#include <mp-units/systems/si.h>
#include <boost/asio.hpp>
#include <boost/ut.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/asio/property.hpp>

#include <tfc/dbus/exception.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/dbus/sdbusplus_meta.hpp>
#include <tfc/progbase.hpp>

using std::string_view_literals::operator""sv;
namespace ut = boost::ut;
namespace asio = boost::asio;

auto main(int argc, char** argv) -> int {
  using ut::operator""_test;
  using ut::expect;

  tfc::base::init(argc, argv);

  "dbus runtime exception test"_test = []() {
    tfc::dbus::exception::runtime t("desc");

    expect(std::string_view(t.what()) == std::string_view("desc"));
    expect(std::string_view(t.description()) == std::string_view("desc"));
    expect(std::string_view(t.name()) == std::string_view("com.skaginn3x.Error.runtimeError"));
    expect(t.get_errno() == 0);
  };

  "sdbusplus meta test"_test = [] {
    enum struct bar { a, b, c };
    std::expected<mp_units::quantity<mp_units::si::gram, std::uint8_t>, bar> hello{ 10 * mp_units::si::gram };
    asio::io_context ctx;
    auto conn{ std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system()) };
    conn->request_name("com.skaginn3x.test");
    sdbusplus::asio::dbus_interface iface{ conn, "/com/skaginn3x/test", "com.skaginn3x.test" };
    using hello_t = decltype(hello);
    iface.register_property_rw<hello_t>(
        "name", sdbusplus::vtable::property_::emits_change,
        []([[maybe_unused]] hello_t const& req, [[maybe_unused]] hello_t const& old) -> int { return 1; },
        [&hello]([[maybe_unused]] hello_t const& foo) -> hello_t { return hello; });
    iface.initialize();

    bool ran{};
    sdbusplus::asio::getProperty<hello_t>(*conn, "com.skaginn3x.test", "/com/skaginn3x/test", "com.skaginn3x.test", "name",
                                          [&ctx, &hello, &ran](auto err, [[maybe_unused]] hello_t const& value) {
                                            if (err) {
                                              fmt::println("Error from getter: {}", err.message());
                                              return;
                                            }
                                            expect(hello == value);
                                            ctx.stop();
                                            ran = true;
                                          });
    ctx.run_for(std::chrono::milliseconds{ 100 });
    expect(ran);
  };
}
