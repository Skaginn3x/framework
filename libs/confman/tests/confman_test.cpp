#include <boost/ut.hpp>
#include <boost/asio.hpp>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>

namespace asio = boost::asio;
namespace ut = boost::ut;
using ut::operator""_test;
using tfc::confman::observable;
using tfc::confman::config;

struct storage {
  observable<int> a{33};
  observable<int> b{44};
  observable<std::string> c{"c"};
};

auto main(int, char**) -> int {
  [[maybe_unused]] ut::suite const rpc_test_cases = [] {
    "happy confman"_test = [] {
      asio::io_context ctx{};
      config<storage> config_storage{ctx, "my-key"};
    };
  };
  return static_cast<int>(boost::ut::cfg<>.run({ .report_errors = true }));
}
