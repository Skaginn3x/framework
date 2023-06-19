// Copyright Takatoshi Kondo 2023
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>
#include <vector>

#include <boost/asio.hpp>

#include <async_mqtt/all.hpp>

namespace as = boost::asio;
namespace am = async_mqtt;

#include <boost/asio/yield.hpp>

struct app {
  app(as::ip::tcp::resolver& res,
      std::string host,
      std::string port,
      am::endpoint<am::role::client, am::protocol::mqtt>& amep,
      std::vector<std::string> signals_on_client
      ):res{res},
        host{std::move(host)},
        port{std::move(port)},
        amep{amep},
        signals_on_client{std::move(signals_on_client)}
  {}

  // forwarding callbacks
  void operator()() { proc({}, {}, {}, {}); }
  void operator()(boost::system::error_code const& ec) { proc(ec, {}, {}, {}); }
  void operator()(boost::system::error_code ec, as::ip::tcp::resolver::results_type eps) {
    proc(ec, {}, {}, std::move(eps));
  }
  void operator()(boost::system::error_code ec, as::ip::tcp::endpoint /*unused*/) { proc(ec, {}, {}, {}); }
  void operator()(am::system_error const& se) { proc({}, se, {}, {}); }
  void operator()(am::packet_variant pv) { proc({}, {}, am::force_move(pv), {}); }

private:
  void proc(boost::system::error_code const& ec,
            am::system_error const& se,
            am::packet_variant pv,
            std::optional<as::ip::tcp::resolver::results_type> eps) {
    reenter(coro) {
      // Resolve hostname
      yield res.async_resolve(host, port, *this);
      if (ec)
        return;

      // Layer
      // am::stream -> TCP

      // Underlying TCP connect
      yield as::async_connect(amep.next_layer(), *eps, *this);

      if (ec)
        return;

      // Send MQTT CONNECT
      yield amep.send(
          am::v3_1_1::connect_packet{
              true,         // clean_session
              0x1234,       // keep_alive
              am::allocate_buffer("cid1"),
              am::nullopt,  // will
              am::nullopt,  // username set like am::allocate_buffer("user1"),
              am::nullopt   // password set like am::allocate_buffer("pass1")
          },
          *this);
      if (se) {
        return;
      }

      // Recv MQTT CONNACK
      yield amep.recv(*this);
      if (pv) {
        pv.visit(am::overload{ [&](am::v3_1_1::connack_packet const& p) {
                                std::cout << "MQTT CONNACK recv"
                                          << " sp:" << p.session_present() << std::endl;
                              },
                               [](auto const&) {} });
      } else {
        std::cout << "MQTT CONNACK recv error:" << pv.get<am::system_error>().what() << std::endl;
        return;
      }

      // Send MQTT SUBSCRIBE
      yield amep.send(am::v3_1_1::subscribe_packet{ *amep.acquire_unique_packet_id(),
                                                    { { am::allocate_buffer("signal_topic"), am::qos::at_most_once } } },
                      *this);
      if (se) {
        std::cout << "MQTT SUBSCRIBE send error:" << se.what() << std::endl;
        return;
      }
      // Recv MQTT SUBACK
      yield amep.recv(*this);
      if (pv) {
        pv.visit(am::overload{ [&](am::v3_1_1::suback_packet const& p) {
                                std::cout << "MQTT SUBACK recv"
                                          << " pid:" << p.packet_id() << " entries:";
                                for (auto const& e : p.entries()) {
                                  std::cout << e << " ";
                                }
                                std::cout << std::endl;
                              },
                               [](auto const&) {} });
      } else {
        std::cout << "MQTT SUBACK recv error:" << pv.get<am::system_error>().what() << std::endl;
        return;
      }
      // Send MQTT PUBLISH
      yield amep.send(am::v3_1_1::publish_packet{ *amep.acquire_unique_packet_id(), am::allocate_buffer("signal_topic"),
                                                  am::allocate_buffer("signal_payload"), am::qos::at_least_once },
                      *this);
      if (se) {
        std::cout << "MQTT PUBLISH send error:" << se.what() << std::endl;
        return;
      }

      // Recv MQTT PUBLISH and PUBACK (order depends on broker)
      // for (count = 0; count != 2; ++count) {
      //  yield amep.recv(*this);
      //  if (pv) {
      //    pv.visit(
      //        am::overload {
      //            [&](am::v3_1_1::publish_packet const& p) {
      //              std::cout
      //                  << "MQTT PUBLISH recv"
      //                  << " pid:" << p.packet_id()
      //                  << " topic:" << p.topic()
      //                  << " payload:" << am::to_string(p.payload())
      //                  << " qos:" << p.opts().get_qos()
      //                  << " retain:" << p.opts().get_retain()
      //                  << " dup:" << p.opts().get_dup()
      //                  << std::endl;
      //            },
      //            [&](am::v3_1_1::puback_packet const& p) {
      //              std::cout
      //                  << "MQTT PUBACK recv"
      //                  << " pid:" << p.packet_id()
      //                  << std::endl;
      //            },
      //            [](auto const&) {}
      //        }
      //    );
      //  }
      //  else {
      //    std::cout
      //        << "MQTT recv error:"
      //        << pv.get<am::system_error>().what()
      //        << std::endl;
      //    return;
      //  }
      //}
      // std::cout << "close" << std::endl;
      // yield amep.close(*this);
    }
  }

  as::ip::tcp::resolver& res;
  std::string host;
  std::string port;
  am::endpoint<am::role::client, am::protocol::mqtt>& amep;
  std::vector<std::string> signals_on_client;
  std::size_t count = 0;
  as::coroutine coro;
};




















#include <boost/asio/unyield.hpp>

auto main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) -> int {
  as::io_context ioc;
  //std::vector<std::string> signals_on_client = available_signals(ioc, argc, argv);
  as::ip::tcp::socket resolve_sock{ ioc };
  as::ip::tcp::resolver res{ resolve_sock.get_executor() };
  am::endpoint<am::role::client, am::protocol::mqtt> amep{ am::protocol_version::v3_1_1, ioc.get_executor() };
  app a{ res, "localhost", "1883", amep, signals_on_client };
  //app a{ res, "localhost", "1883", amep };
  a();
  ioc.run();
}