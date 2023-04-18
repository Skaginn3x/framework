#include <boost/ut.hpp>
#include <glaze/glaze.hpp>
#include <tfc/confman/read_only.hpp>

#include <fmt/core.h>

struct read_write_struct {
  tfc::confman::read_only<int> read_only_int{ 1337 };
  int read_write_int{ 42 };
  struct glaze {
    static auto constexpr value{
      glz::object("read_only_int", &read_write_struct::read_only_int, "read_write_int", &read_write_struct::read_write_int)
    };
  };
};

auto main(int, char**) -> int {
  using namespace boost::ut;

  "Write to json read_only"_test = []() {
    tfc::confman::read_only<int> const test_me{ 1337 };
    auto json_str{ glz::write_json(test_me) };
    expect(json_str == "1337");
  };

  "Block read from json read_only"_test = []() {
    read_write_struct foo{ .read_only_int = tfc::confman::read_only<int>{ 1 }, .read_write_int = 2 };
    auto json_str{ glz::write_json(foo) };
    expect(json_str == R"({"read_only_int":1,"read_write_int":2})");
    constexpr std::string_view json_modded{ R"({"read_only_int":3,"read_write_int":5})" };
    [[maybe_unused]] auto parse_error{ glz::read_json<read_write_struct>(foo, json_modded) };
    fmt::print("{}\n", glz::format_error(parse_error, json_modded));
    expect(foo.read_only_int == 1);
    expect(foo.read_write_int == 5);
  };
}
