
#include <tfc/dbus/match_rules.hpp>

namespace tfc::dbus::match::rules::test {

static constexpr std::string_view foo{ "foo" };
static_assert("sender='foo'," == sender<foo>);
static_assert("interface='foo'," == interface<foo>);
static_assert("member='foo'," == member<foo>);
static_assert("path='foo'," == path<foo>);
static_assert("path_namespace='foo'," == path_namespace<foo>);
static_assert("destination='foo'," == destination<foo>);

}  // namespace tfc::dbus::match::rules::test

int main() {
  return 0;
}
