#include <optional>
#include <thread>

#include <boost/process.hpp>

#include <boost/test/unit_test.hpp>

namespace as = boost::asio;
namespace pr = boost::process;

struct broker_runner {
  broker_runner() {
    brk.emplace(pr::args({ "broker" }));
    {
      as::io_context ioc;
      as::ip::address address = boost::asio::ip::make_address("127.0.0.1");
      as::ip::tcp::endpoint endpoint{ address, 1883 };
      as::ip::tcp::socket s{ ioc };
      std::function<void(boost::system::error_code const&)> f = [&](boost::system::error_code const& ec) {
        if (ec) {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          s = as::ip::tcp::socket{ ioc };
          s.async_connect(endpoint, f);
        }
      };
      s.async_connect(endpoint, f);
      ioc.run();
    }
  }
  ~broker_runner() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (brk)
      brk->join();
  }
  std::optional<pr::child> brk;
};
