#include <boost/ut.hpp>
#include <string_view>

#include <tfc/progbase.hpp>
#include <tfc/snitch.hpp>
#include <glaze/util/string_literal.hpp>

using std::string_view_literals::operator""sv;

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
// todo this below shouldn't be possible, remove if causes compile error and celebrate
static_assert(arg_names<glz::chars<"foo {name} {name}">>()[1] == "");

static_assert(arg_names<glz::chars<"foo {name} {name} {bar}">>()[1] == "bar");
static_assert(arg_names<glz::chars<"foo {name} {name} {bar}">>().size() == 2);

}  // namespace test

auto main(int argc, char** argv) -> int {
  using boost::ut::operator""_test;
  using boost::ut::expect;

  tfc::base::init(argc, argv);

  "snitches"_test = [] {
    tfc::snitch::info<"short desc {name}", "long desc {name} {index}"> tank(fmt::arg("name", "hello"), fmt::arg("index", 42));
    tank.set();

    // warn.on_ack([]{});

  };
}
