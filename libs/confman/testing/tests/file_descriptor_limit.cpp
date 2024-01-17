#include <filesystem>
#include <sys/resource.h>

#include <boost/asio.hpp>
#include <boost/ut.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <fmt/core.h>

#include <tfc/confman.hpp>

namespace asio = boost::asio;
namespace ut = boost::ut;
using ut::operator""_test;
using ut::operator/;

template <typename storage_t>
class config_testable : public tfc::confman::config<storage_t> {
public:
  using tfc::confman::config<storage_t>::config;

  ~config_testable() {
    std::error_code ignore{};
    std::filesystem::remove(this->file(), ignore);
  }
};

static auto print_fd_limit() -> void {
  rlimit limit{};
  auto const res{ getrlimit(RLIMIT_NOFILE, &limit) };
  if (res != 0) {
    fmt::println(stderr, "Failed to getrlimit: {}", std::strerror(errno));
    return;
  }
  fmt::println("File descriptor soft limit: {}", limit.rlim_cur);
  fmt::println("File descriptor hard limit: {}", limit.rlim_max);
}

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);


  // no reason for the config complexity, just to test in general
  using config_t = std::vector<std::map<std::string, std::string>>;

  "make many many instances of config"_test = [&] {
    asio::io_context ctx{};
    print_fd_limit();
    std::vector<std::shared_ptr<config_testable<config_t>>> instances{};
    // this line in sdbusplus  _intf->sd_bus_get_unique_name(_bus.get(), &unique);
    // returns null as unique name for the 241 iteration
    for (auto i = 0; i < 240; ++i) {
      instances.emplace_back(std::make_shared<config_testable<config_t>>(ctx, fmt::format("foo{}", i), config_t{ { { "a", "1" }, { "b", "2" }, { "c", "bar" } } }));
      ctx.run_for(std::chrono::milliseconds{ 1 });
    }
    fmt::println("All done");
  };
  return EXIT_SUCCESS;
}
