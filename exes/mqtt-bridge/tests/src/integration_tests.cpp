#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <thread>

#include <async_mqtt/broker/broker.hpp>
#include <async_mqtt/broker/constant.hpp>
#include <async_mqtt/broker/endpoint_variant.hpp>
#include <async_mqtt/broker/fixed_core_map.hpp>
#include <async_mqtt/predefined_underlying_layer.hpp>
#include <async_mqtt/setup_log.hpp>
#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/format.hpp>
#include <boost/process.hpp>
#include <boost/program_options.hpp>
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <string>

#include <boost/asio.hpp>

#include <async_mqtt/all.hpp>

namespace asio = boost::asio;
namespace pr = boost::process;
namespace am = async_mqtt;

class broker {
public:
  static void run_broker() {
    try {
      asio::io_context timer_ioc;

      using epv_t = am::endpoint_variant<am::role::server, am::protocol::mqtt>;

      am::broker<epv_t> brk{ timer_ioc };

      auto num_of_iocs = 1;

      std::size_t num_of_cores = std::thread::hardware_concurrency();

      std::size_t threads_per_ioc = 1;

      std::cout << "iocs:" << num_of_iocs << " threads_per_ioc:" << threads_per_ioc
                << " total threads:" << num_of_iocs * threads_per_ioc;

      asio::io_context accept_ioc;

      std::mutex mtx_con_iocs;
      std::vector<asio::io_context> con_iocs(num_of_iocs);
      BOOST_ASSERT(!con_iocs.empty());

      std::vector<asio::executor_work_guard<asio::io_context::executor_type>> guard_con_iocs;
      guard_con_iocs.reserve(con_iocs.size());
      for (auto& con_ioc : con_iocs) {
        guard_con_iocs.emplace_back(con_ioc.get_executor());
      }

      auto con_iocs_it = con_iocs.begin();

      auto con_ioc_getter = [&mtx_con_iocs, &con_iocs, &con_iocs_it]() -> asio::io_context& {
        std::lock_guard<std::mutex> g{ mtx_con_iocs };
        auto& ret = *con_iocs_it++;
        if (con_iocs_it == con_iocs.end())
          con_iocs_it = con_iocs.begin();
        return ret;
      };

      // mqtt (MQTT on TCP)
      am::optional<asio::ip::tcp::endpoint> mqtt_endpoint;
      am::optional<asio::ip::tcp::acceptor> mqtt_ac;
      std::function<void()> mqtt_async_accept;
      mqtt_endpoint.emplace(asio::ip::tcp::v4(), 1883);
      mqtt_ac.emplace(accept_ioc, *mqtt_endpoint);
      mqtt_async_accept = [&] {
        auto epsp = am::endpoint<am::role::server, am::protocol::mqtt>::create(am::protocol_version::undetermined,
                                                                               con_ioc_getter().get_executor());

        auto& lowest_layer = epsp->lowest_layer();
        mqtt_ac->async_accept(lowest_layer, [&mqtt_async_accept, &brk, epsp](boost::system::error_code const& ec) mutable {
          if (ec) {
            std::cout << "TCP accept error:" << ec.message();
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
          std::cout << "th_accept exception:" << e.what();
        }
        std::cout << "accept_ioc.run() finished";
      } };

      asio::executor_work_guard<asio::io_context::executor_type> guard_timer_ioc(timer_ioc.get_executor());

      std::thread th_timer{ [&timer_ioc] {
        try {
          timer_ioc.run();
        } catch (std::exception const& e) {
          std::cout << "th_timer exception:" << e.what();
        }
        std::cout << "timer_ioc.run() finished";
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
              std::cout << "th con exception:" << e.what();
            }
            std::cout << "con_ioc.run() finished";
          });
        }
        ++ioc_index;
      }

      asio::io_context ioc_signal;
      asio::signal_set signals{ ioc_signal, SIGINT, SIGTERM };
      signals.async_wait([](boost::system::error_code const& ec, int num) {
        if (!ec) {
          std::cout << "Signal " << num << " received. exit program";
          exit(-1);
        }
      });
      std::thread th_signal{ [&] {
        try {
          ioc_signal.run();
        } catch (std::exception const& e) {
          std::cout << "th_signal exception:" << e.what();
        }
      } };

      th_accept.join();
      std::cout << "th_accept joined";

      for (auto& g : guard_con_iocs)
        g.reset();
      for (auto& t : ts)
        t.join();
      std::cout << "ts joined";

      guard_timer_ioc.reset();
      th_timer.join();
      std::cout << "th_timer joined";

      signals.cancel();
      th_signal.join();
      std::cout << "th_signal joined";
    } catch (std::exception const& e) {
      std::cout << e.what();
    }
  }
};

auto run_broker(asio::io_context&) -> void {}

auto main() -> int {
  asio::io_context io_ctx{};

  using epv_t = am::endpoint_variant<am::role::server, am::protocol::mqtt>;

  am::broker<epv_t> brk{ io_ctx };

  // mqtt (MQTT on TCP)
  asio::ip::tcp::endpoint mqtt_endpoint{ asio::ip::tcp::v4(), 1883 };

  asio::ip::tcp::acceptor mqtt_ac{ io_ctx, mqtt_endpoint };

  std::function<void()> mqtt_async_accept = [&] {
    auto epsp = am::endpoint<am::role::server, am::protocol::mqtt>::create(am::protocol_version::undetermined,
                                                                           io_ctx.get_executor());

    auto& lowest_layer = epsp->lowest_layer();
    mqtt_ac.async_accept(lowest_layer, [&mqtt_async_accept, &brk, epsp](boost::system::error_code const& ec) mutable {
      if (ec) {
        std::cout << "TCP accept error:" << ec.message();
      } else {
        brk.handle_accept(epv_t{ force_move(epsp) });
      }
      mqtt_async_accept();
    });
  };

  mqtt_async_accept();

  // asio::executor_work_guard<asio::io_context::executor_type> guard_timer_ioc(io_ctx.get_executor());

  //  asio::io_context ioc_signal;
  //  asio::signal_set signals{ ioc_signal, SIGINT, SIGTERM };
  //  signals.async_wait([](boost::system::error_code const& ec, int num) {
  //    if (!ec) {
  //      std::cout << "Signal " << num << " received. exit program";
  //      exit(-1);
  //    }
  //  });

  //   std::thread th_signal{ [&] {
  //     try {
  //       ioc_signal.run();
  //     } catch (std::exception const& e) {
  //       std::cout << "th_signal exception:" << e.what();
  //     }
  //   } };

  //  signals.cancel();
  //  th_signal.join();
  // std::cout << "th_signal joined";

  // ------------------------------------------------------------------------------------------------------------------------------------------------

  //   std::cout << "after run broker " << std::endl;
  //
  //   // io_ctx.run_for(std::chrono::seconds{ 10 });
  //   std::this_thread::sleep_for(std::chrono::seconds{ 1 });
  //
  //   std::cout << "after sleep thread" << std::endl;
  //
  //   // std::thread client{ &client::subscribe };
  //
  //   std::this_thread::sleep_for(std::chrono::seconds{ 1 });
  //
  //   asio::ip::tcp::socket resolve_sock{ io_ctx };
  //   asio::ip::tcp::resolver res{ resolve_sock.get_executor() };
  //   auto amep = am::endpoint<am::role::client, am::protocol::mqtt>::create(am::protocol_version::v5,
  //   io_ctx.get_executor());
  //
  //   std::cout << "start" << std::endl;
  //   // std::size_t count = 0;
  //
  //   // Resolve hostname
  //   res.async_resolve("127.0.0.1", "1883", [&](boost::system::error_code ec, asio::ip::tcp::resolver::results_type eps) {
  //     // res.async_resolve("0.0.0.0", "1883", [&](boost::system::error_code ec, asio::ip::tcp::resolver::results_type eps)
  //     std::cout << "async_resolve:" << ec.message() << std::endl;
  //     if (ec)
  //       return;
  //     // Layer
  //     // am::stream -> TCP
  //
  //     // Underlying TCP connect
  //     asio::async_connect(amep->next_layer(), eps, [&](boost::system::error_code ec, asio::ip::tcp::endpoint /*unused*/) {
  //       std::cout << "TCP connected ec:" << ec.message() << std::endl;
  //       if (ec)
  //         return;
  //       amep->send(
  //           am::v5::connect_packet{
  //               true,
  //               0x1234,
  //               am::allocate_buffer("cid2"),
  //               // am::allocate_buffer("cid1"),
  //               am::nullopt,
  //               am::nullopt,
  //               am::nullopt,
  //           },
  //           [&](am::system_error const& se) {
  //             if (se) {
  //               std::cout << "MQTT CONNECT send error:" << se.what() << std::endl;
  //               return;
  //             }
  //             // Recv MQTT CONNACK
  //             amep->recv([&](am::packet_variant pv) {
  //               if (pv) {
  //                 pv.visit(am::overload{
  //                     [&](am::v5::connack_packet const& p) {
  //                       std::cout << "MQTT CONNACK recv"
  //                                 << " sp:" << p.session_present() << std::endl;
  //                       // Send MQTT SUBSCRIBE
  //                       amep->send(
  //                           am::v5::subscribe_packet{ *amep->acquire_unique_packet_id(),
  //                                                     // { { am::allocate_buffer("hello"), am::qos::at_most_once } } },
  //                                                     { { am::allocate_buffer("#"), am::qos::at_most_once } } },
  //                           [&](am::system_error const& se) {
  //                             if (se) {
  //                               std::cout << "MQTT SUBSCRIBE send error:" << se.what() << std::endl;
  //                               return;
  //                             }
  //                             // Recv MQTT SUBACK
  //                             amep->recv([&](am::packet_variant pv) {
  //                               if (pv) {
  //                                 pv.visit(am::overload{
  //                                     [&](am::v5::suback_packet const& p) {
  //                                       std::cout << "MQTT SUBACK recv"
  //                                                 << " pid:" << p.packet_id() << " entries:";
  //                                       for (auto const& e : p.entries()) {
  //                                         std::cout << e << " ";
  //                                       }
  //                                       std::cout << std::endl;
  //                                       // Send MQTT PUBLISH
  //                                       amep->send(
  //                                           am::v5::publish_packet{ *amep->acquire_unique_packet_id(),
  //                                                                   am::allocate_buffer("hello"),
  //                                                                   am::allocate_buffer("hello"), am::qos::at_least_once
  //                                                                   },
  //                                           [&](am::system_error const& se) {
  //                                             if (se) {
  //                                               std::cout << "MQTT PUBLISH send error:" << se.what() << std::endl;
  //                                               return;
  //                                             }
  //
  //                                             // Recv MQTT PUBLISH and PUBACK (order depends on broker)
  //                                             auto recv_handler =
  //                                                 std::make_shared<std::function<void(am::packet_variant pv)>>();
  //                                             *recv_handler = [&, recv_handler](am::packet_variant pv) {
  //                                               if (pv) {
  //                                                 pv.visit(am::overload{
  //                                                     [&](am::v5::publish_packet const& p) {
  //                                                       std::cout << "received: topic {" << p.topic() << "} payload {"
  //                                                                 << am::to_string(p.payload()) << "}" << std::endl;
  //                                                     },
  //                                                     //  [&](am::v5::puback_packet const& p) {
  //                                                     //    std::cout << "MQTT PUBACK recv"
  //                                                     //              << " pid:" << p.packet_id() << std::endl;
  //                                                     //  },
  //                                                     [](auto const&) {} });
  //                                                 amep->recv(*recv_handler);
  //                                               } else {
  //                                                 std::cout << "MQTT recv error:" << pv.get<am::system_error>().what()
  //                                                           << std::endl;
  //                                                 return;
  //                                               }
  //                                             };
  //                                             amep->recv(*recv_handler);
  //                                           });
  //                                     },
  //                                     [](auto const&) {} });
  //                               } else {
  //                                 std::cout << "MQTT SUBACK recv error:" << pv.get<am::system_error>().what() <<
  //                                 std::endl; return;
  //                               }
  //                             });
  //                           });
  //                     },
  //                     [](auto const&) {} });
  //               } else {
  //                 std::cout << "MQTT CONNACK recv error:" << pv.get<am::system_error>().what() << std::endl;
  //                 return;
  //               }
  //             });
  //           });
  //     });
  //   });
  //
  //   std::cout << "before ctx" << std::endl;
  //
  //   io_ctx.run_for(std::chrono::seconds{ 1 });
  //
  //   std::cout << "before join" << std::endl;
  // brk.join();
  io_ctx.run();

  return 0;
}
