#include <boost/asio.hpp>
#include <boost/ut.hpp>
#include <glaze/glaze.hpp>
#include <tfc/confman/file_storage.hpp>

#include <tfc/progbase.hpp>

struct test_me {
  int a{};
  std::string b{};
  struct glaze {
    static constexpr auto value{ glz::object("a", &test_me::a, "b", &test_me::b) };
    static constexpr auto name{ "test_me" };
  };
};
[[maybe_unused]] static auto operator==(test_me const& lhs, test_me const& rhs) noexcept -> bool {
  return lhs.a == rhs.a && lhs.b == rhs.b;
}

using map = std::map<std::string, test_me>;

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  using namespace boost::ut;

  "change test"_test = []() {
    std::filesystem::remove("/tmp/test.me");
    boost::asio::io_context ctx{};
    tfc::confman::file_storage<map> my_map{ ctx, "/tmp/test.me" };
    { my_map.make_change().value()["new_key"] = test_me{ .a = 42, .b = "hello-world" }; }
    expect(my_map.value().at("new_key") == test_me{ .a = 42, .b = "hello-world" });
    expect(my_map->at("new_key") == test_me{ .a = 42, .b = "hello-world" });

    my_map.on_change([]() { fmt::print("changed\n"); });

    //    ctx.run();
  };

  return EXIT_SUCCESS;
}
