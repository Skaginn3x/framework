// Copyright Takatoshi Kondo 2023
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <thread>
#include <optional>

#include <boost/process.hpp>
#include <boost/process/args.hpp>
#include <boost/asio.hpp>
#include <boost/predef.h>

#include <boost/test/unit_test.hpp>

namespace as = boost::asio;
namespace pr = boost::process;
// namespace am = async_mqtt;

// inline bool launch_broker_required() {
//     auto argc = boost::unit_test::framework::master_test_suite().argc;
//     if (argc >= 3) {
//         auto argv = boost::unit_test::framework::master_test_suite().argv;
//         auto launch = std::string_view(argv[2]);
//         if (launch == "no" || launch == "no_launch") {
//             return false;
//         }
//     }
//     return true;
// }

inline void kill_broker() {
    // if (!launch_broker_required()) return;
    // std::system("pkill broker");
}

struct broker_killer {
    broker_killer() {
        kill_broker();
    }
};

struct broker_runner : broker_killer {
    broker_runner(
       //  std::string const& config = "st_broker.conf",
       //  std::string const& auth = "st_auth.json"
    ) {
        // if (launch_broker_required()) {
        // brk.emplace(pr::args({"../../../tool/broker"}));
        brk.emplace(pr::args({"broker"}));
        // }
        {
            as::io_context ioc;
            as::ip::address address = boost::asio::ip::make_address("127.0.0.1");
            as::ip::tcp::endpoint endpoint{address, 1883};
            as::ip::tcp::socket s{ioc};
            std::function<void(boost::system::error_code const&)> f =
                [&](boost::system::error_code const& ec) {
                    if (ec) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        s = as::ip::tcp::socket{ioc};
                        s.async_connect(
                            endpoint,
                            f
                        );
                    }
                };
            s.async_connect(
                endpoint,
                f
            );
            ioc.run();
        }
    }
    ~broker_runner() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        kill_broker();
        if (brk) brk->join();
    }
    std::optional<pr::child> brk;
};
