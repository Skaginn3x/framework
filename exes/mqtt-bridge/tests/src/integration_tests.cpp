// #include <algorithm>
// #include <fstream>
// #include <iomanip>
// #include <iostream>
// #include <optional>
// #include <stdexcept>
// #include <thread>

#include <async_mqtt/broker/broker.hpp>
// #include <async_mqtt/broker/constant.hpp>
#include <async_mqtt/broker/endpoint_variant.hpp>
// #include <async_mqtt/broker/fixed_core_map.hpp>
// #include <async_mqtt/predefined_underlying_layer.hpp>
// #include <async_mqtt/setup_log.hpp>
#include <boost/asio.hpp>
// #include <boost/asio/signal_set.hpp>
// #include <boost/format.hpp>
// #include <boost/process.hpp>
// #include <boost/program_options.hpp>
// #include <boost/test/unit_test.hpp>
//
#include <iostream>
// #include <string>
//
// #include <boost/asio.hpp>
//
#include <async_mqtt/all.hpp>

namespace asio = boost::asio;
// namespace pr = boost::process;
namespace am = async_mqtt;

struct message {
  std::string topic;
  std::string payload;
};

auto main() -> int {
  asio::io_context io_ctx{};

  std::vector<message> messages;

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

  // std::this_thread::sleep_for(std::chrono::seconds{ 1 });

  io_ctx.run_for(std::chrono::milliseconds{100});

  asio::ip::tcp::socket resolve_sock{ io_ctx };
  asio::ip::tcp::resolver res{ resolve_sock.get_executor() };
  auto amep = am::endpoint<am::role::client, am::protocol::mqtt>::create(am::protocol_version::v5, io_ctx.get_executor());

  res.async_resolve("127.0.0.1", "1883", [&](boost::system::error_code ec, asio::ip::tcp::resolver::results_type eps) {
    if (ec)
      return;

    asio::async_connect(amep->next_layer(), eps, [&](boost::system::error_code ec, asio::ip::tcp::endpoint /*unused*/) {
     //  std::cout << "TCP connected ec:" << ec.message() << std::endl;
     //  if (ec)
     //    return;
      amep->send(
          am::v5::connect_packet{
              true,
              0x1234,
              am::allocate_buffer("cid2"),
              am::nullopt,
              am::nullopt,
              am::nullopt,
          },
          [&](am::system_error const& se) {
            if (se) {
              std::cout << "MQTT CONNECT send error:" << se.what() << std::endl;
              return;
            }
            // Recv MQTT CONNACK
            amep->recv([&](am::packet_variant pv) {
              // if (pv) {
              pv.visit(am::overload{
                  [&](am::v5::connack_packet const&) {
                    amep->send(
                        am::v5::subscribe_packet{
                            *amep->acquire_unique_packet_id(),
                            { { am::allocate_buffer("spBv1.0/tfc_unconfigured_group_id/NDATA/tfc_unconfigured_node_id"),
                                am::qos::at_most_once } } },
                        [&](am::system_error const& ) {
                          //  if (se) {
                          //    std::cout << "MQTT SUBSCRIBE send error:" << se.what() << std::endl;
                          //    return;
                          //  }
                          // Recv MQTT SUBACK
                          amep->recv([&](am::packet_variant pv) {
                            // if (pv) {
                            pv.visit(am::overload{ [&](am::v5::suback_packet const& p) {
                                                    std::cout << "MQTT SUBACK recv"
                                                              << " pid:" << p.packet_id() << " entries:";

                                                    auto recv_handler =
                                                        std::make_shared<std::function<void(am::packet_variant pv)>>();
                                                    *recv_handler = [&, recv_handler](am::packet_variant pv) {
                                                      // if (pv) {
                                                      pv.visit(am::overload{ [&](am::v5::publish_packet const& p) {
                                                                              messages.emplace_back(p.topic().data(),
                                                                                                    p.payload()[0].data());
                                                                            },
                                                                             [](auto const&) {} });
                                                      // if (messages.size() < 3) {
                                                      //   amep->recv(*recv_handler);
                                                      // }
                                                      // } else {
                                                      //   std::cout << "MQTT recv error:" <<
                                                      //   pv.get<am::system_error>().what() << std::endl; return;
                                                      // }
                                                    };
                                                    amep->recv(*recv_handler);
                                                    // });
                                                  },
                                                   [](auto const&) {} });
                            //  } else {
                            //    std::cout << "MQTT SUBACK recv error:" << pv.get<am::system_error>().what() << std::endl;
                            //    return;
                            //  }
                          });
                        });
                  },
                  [](auto const&) {} });
              //   } else {
              //     std::cout << "MQTT CONNACK recv error:" << pv.get<am::system_error>().what() << std::endl;
              //     return;
              //   }
            });
          });
    });
  });

  io_ctx.run_for(std::chrono::seconds{ 10 });

  for (auto& m : messages) {
    std::cout << "topic: " << m.topic << " payload: " << m.payload << std::endl;
  }

  return 0;
}
