#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <thread>

#include <boost/asio/signal_set.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <async_mqtt/broker/broker.hpp>
#include <async_mqtt/broker/constant.hpp>
#include <async_mqtt/broker/endpoint_variant.hpp>
#include <async_mqtt/broker/fixed_core_map.hpp>
#include <async_mqtt/predefined_underlying_layer.hpp>
#include <async_mqtt/setup_log.hpp>

namespace am = async_mqtt;
namespace as = boost::asio;

inline void run_broker() {
  try {
    as::io_context timer_ioc;

    using epv_t = am::endpoint_variant<am::role::server, am::protocol::mqtt>;

    am::broker<epv_t> brk{ timer_ioc };

    auto num_of_iocs = 1;

    std::size_t num_of_cores = std::thread::hardware_concurrency();

    std::size_t threads_per_ioc = 1;

    ASYNC_MQTT_LOG("mqtt_broker", info) << "iocs:" << num_of_iocs << " threads_per_ioc:" << threads_per_ioc
                                        << " total threads:" << num_of_iocs * threads_per_ioc;

    std::string auth_file = R"(

         {
 # Configure username/login
             "authentication": [
             {
                 "name": "u1",
                         "method": "plain_password",
                         "password": "passforu1"
             }
             ,
             {
 # Authenticates user by client certificate
                 "name": "cid1",
                         "method": "client_cert"
             }
             ,
             {
                 "name": "u3",
                         "method": "sha256",
                         "salt": "mysalt",
 # mysaltpassforu3
 # you can get it `echo -n mysaltpassforu3 | sha256sum`
 # don't forget -n
                         "digest": "9ed77f119f694e5e543201a97cc8db226cf36c814c4406fe2eac5b9b1f084ba9"
             }
             ]
             ,
 # Give access to topics
             "authorization": [
             {
 # Specified users and groups are denied to subscribe and publish on this topic
                 "topic": "#",
                         "allow": {
                     "sub": ["u1"],
                     "pub": ["u1"]
                 }
             }
             ]
         }
        )";

    if (!auth_file.empty()) {
      ASYNC_MQTT_LOG("mqtt_broker", info) << "auth_file:" << auth_file;

      std::ifstream input(auth_file);

      if (input) {
        am::security security;
        security.load_json(input);
        brk.set_security(am::force_move(security));
      } else {
        ASYNC_MQTT_LOG("mqtt_broker", warning)
            << "Authorization file '" << auth_file << "' not found,  broker doesn't use authorization file.";
      }
    }

    as::io_context accept_ioc;

    std::mutex mtx_con_iocs;
    std::vector<as::io_context> con_iocs(num_of_iocs);
    BOOST_ASSERT(!con_iocs.empty());

    std::vector<as::executor_work_guard<as::io_context::executor_type> > guard_con_iocs;
    guard_con_iocs.reserve(con_iocs.size());
    for (auto& con_ioc : con_iocs) {
      guard_con_iocs.emplace_back(con_ioc.get_executor());
    }

    auto con_iocs_it = con_iocs.begin();

    auto con_ioc_getter = [&mtx_con_iocs, &con_iocs, &con_iocs_it]() -> as::io_context& {
      std::lock_guard<std::mutex> g{ mtx_con_iocs };
      auto& ret = *con_iocs_it++;
      if (con_iocs_it == con_iocs.end())
        con_iocs_it = con_iocs.begin();
      return ret;
    };

    // mqtt (MQTT on TCP)
    am::optional<as::ip::tcp::endpoint> mqtt_endpoint;
    am::optional<as::ip::tcp::acceptor> mqtt_ac;
    std::function<void()> mqtt_async_accept;
    mqtt_endpoint.emplace(as::ip::tcp::v4(), 1883);
    mqtt_ac.emplace(accept_ioc, *mqtt_endpoint);
    mqtt_async_accept = [&] {
      auto epsp = am::endpoint<am::role::server, am::protocol::mqtt>::create(am::protocol_version::undetermined,
                                                                             con_ioc_getter().get_executor());

      auto& lowest_layer = epsp->lowest_layer();
      mqtt_ac->async_accept(lowest_layer, [&mqtt_async_accept, &brk, epsp](boost::system::error_code const& ec) mutable {
        if (ec) {
          ASYNC_MQTT_LOG("mqtt_broker", error) << "TCP accept error:" << ec.message();
        } else {
          brk.handle_accept(epv_t{ force_move(epsp) });
        }
        mqtt_async_accept();
      });
    };

    mqtt_async_accept();

    std::thread th_accept{ [&accept_ioc] {
      try {
        accept_ioc.run();
      } catch (std::exception const& e) {
        ASYNC_MQTT_LOG("mqtt_broker", error) << "th_accept exception:" << e.what();
      }
      ASYNC_MQTT_LOG("mqtt_broker", trace) << "accept_ioc.run() finished";
    } };

    as::executor_work_guard<as::io_context::executor_type> guard_timer_ioc(timer_ioc.get_executor());

    std::thread th_timer{ [&timer_ioc] {
      try {
        timer_ioc.run();
      } catch (std::exception const& e) {
        ASYNC_MQTT_LOG("mqtt_broker", error) << "th_timer exception:" << e.what();
      }
      ASYNC_MQTT_LOG("mqtt_broker", trace) << "timer_ioc.run() finished";
    } };
    std::vector<std::thread> ts;
    ts.reserve(num_of_iocs * threads_per_ioc);

    auto fixed_core_map = false;

    std::size_t ioc_index = 0;
    for (auto& con_ioc : con_iocs) {
      for (std::size_t i = 0; i != threads_per_ioc; ++i) {
        ts.emplace_back([&con_ioc, ioc_index, num_of_cores, fixed_core_map] {
          try {
            if (fixed_core_map) {
              am::map_core_to_this_thread(ioc_index % num_of_cores);
            }
            con_ioc.run();
          } catch (std::exception const& e) {
            ASYNC_MQTT_LOG("mqtt_broker", error) << "th con exception:" << e.what();
          }
          ASYNC_MQTT_LOG("mqtt_broker", trace) << "con_ioc.run() finished";
        });
      }
      ++ioc_index;
    }

    as::io_context ioc_signal;
    as::signal_set signals{ ioc_signal, SIGINT, SIGTERM };
    signals.async_wait([](boost::system::error_code const& ec, int num) {
      if (!ec) {
        ASYNC_MQTT_LOG("mqtt_broker", trace) << "Signal " << num << " received. exit program";
        exit(-1);
      }
    });
    std::thread th_signal{ [&] {
      try {
        ioc_signal.run();
      } catch (std::exception const& e) {
        ASYNC_MQTT_LOG("mqtt_broker", error) << "th_signal exception:" << e.what();
      }
    } };

    th_accept.join();
    ASYNC_MQTT_LOG("mqtt_broker", trace) << "th_accept joined";

    for (auto& g : guard_con_iocs)
      g.reset();
    for (auto& t : ts)
      t.join();
    ASYNC_MQTT_LOG("mqtt_broker", trace) << "ts joined";

    guard_timer_ioc.reset();
    th_timer.join();
    ASYNC_MQTT_LOG("mqtt_broker", trace) << "th_timer joined";

    signals.cancel();
    th_signal.join();
    ASYNC_MQTT_LOG("mqtt_broker", trace) << "th_signal joined";
  } catch (std::exception const& e) {
    ASYNC_MQTT_LOG("mqtt_broker", error) << e.what();
  }
}

// int main(int argc, char* argv[]) {
int main() {
  try {

    run_broker();
  } catch (std::exception const& e) {
    std::cerr << e.what() << std::endl;
  }
}
