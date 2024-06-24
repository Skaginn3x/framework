#include <boost/ut.hpp>
#include <string_view>

#include <boost/asio.hpp>
#include <glaze/reflection/to_tuple.hpp>
#include <glaze/util/string_literal.hpp>
#include <sdbusplus/asio/connection.hpp>

#include <tfc/dbus/sd_bus.hpp>
#include <tfc/progbase.hpp>
#include <tfc/snitch.hpp>

using std::string_view_literals::operator""sv;
namespace asio = boost::asio;

namespace test {
using tfc::snitch::detail::check_all_arguments_named;
static_assert(check_all_arguments_named(glz::chars<"foo">));
static_assert(check_all_arguments_named(glz::chars<"foo {name}">));
static_assert(!check_all_arguments_named(glz::chars<"foo {}">));
static_assert(!check_all_arguments_named(glz::chars<"foo {:.2f}">));

using tfc::snitch::detail::check_all_arguments_no_format;
static_assert(check_all_arguments_no_format(glz::chars<"foo {name}">));
static_assert(!check_all_arguments_no_format(glz::chars<"foo {name:.2f}">));

using tfc::snitch::detail::arg_count;
static_assert(arg_count(glz::chars<"foo">) == 0);
static_assert(arg_count(glz::chars<"foo {name}">) == 1);
static_assert(arg_count(glz::chars<"foo {name} {name}">) == 2);
static_assert(arg_count(glz::chars<"foo {name} {name} {name}">) == 3);

using tfc::snitch::detail::arg_names;
static_assert(arg_names<glz::chars<"foo {name}">>()[0] == "name");
static_assert(arg_names<glz::chars<"foo {name} {name}">>()[0] == "name");

static_assert(arg_names<glz::chars<"foo {name} {name}">>().size() == 1);
#if __clang__
// todo this below shouldn't be possible, remove if causes compile error and celebrate
static_assert(arg_names<glz::chars<"foo {name} {name}">>()[1] == "");
#endif

static_assert(arg_names<glz::chars<"foo {name} {name} {bar}">>()[1] == "bar");
static_assert(arg_names<glz::chars<"foo {name} {name} {bar}">>().size() == 2);

}  // namespace test

static_assert(glz::detail::count_members<tfc::snitch::api::activation> == 10);

auto main(int argc, char** argv) -> int {
  using boost::ut::operator""_test;
  using boost::ut::expect;

  tfc::base::init(argc, argv);

  "snitches"_test = [] {
    asio::io_context ctx;
    auto connection = std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system());
    tfc::snitch::info<"short desc {name}", "long desc {name} {index}"> tank(
        connection, "unique_id", fmt::arg("name", "hello"), fmt::arg("index", 42));
    tank.set([](auto) {});

    // warn.on_ack([]{});
  };
}
