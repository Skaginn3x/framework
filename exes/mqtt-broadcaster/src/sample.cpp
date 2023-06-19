// Copyright Takatoshi Kondo 2023
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>

#include <boost/asio.hpp>

#include <async_mqtt/all.hpp>

namespace asio = boost::asio;
// namespace am = async_mqtt;

#include <boost/asio/yield.hpp>

struct app {
  app(asio::ip::tcp::resolver& res,
      std::string host,
      std::string port,
      async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtts>& amep)
      : res{ res }, host{ std::move(host) }, port{ std::move(port) }, amep{ amep } {}

  // forwarding callbacks
  void operator()() { proc({}, {}, {}, {}); }
  void operator()(boost::system::error_code const& ec) { proc(ec, {}, {}, {}); }
  void operator()(boost::system::error_code ec, asio::ip::tcp::resolver::results_type eps) {
    proc(ec, {}, {}, std::move(eps));
  }
  void operator()(boost::system::error_code ec, asio::ip::tcp::endpoint /*unused*/) { proc(ec, {}, {}, {}); }
  void operator()(async_mqtt::system_error const& se) { proc({}, se, {}, {}); }
  void operator()(async_mqtt::packet_variant pv) { proc({}, {}, async_mqtt::force_move(pv), {}); }

private:
  void proc(boost::system::error_code const& ec,
            async_mqtt::system_error const& se,
            async_mqtt::packet_variant pv,
            std::optional<asio::ip::tcp::resolver::results_type> eps) {
    reenter(coro) {
      std::cout << "start" << std::endl;

      // Resolve hostname
      yield res.async_resolve(host, port, *this);
      std::cout << "async_resolve:" << ec.message() << std::endl;
      if (ec)
        return;

      // Layer
      // am::stream -> TLS -> TCP

      // Underlying TCP connect
      yield asio::async_connect(amep.next_layer().next_layer(),  // or lowest_layer()
                                *eps, *this);
      std::cout << "TCP connected ec:" << ec.message() << std::endl;

      if (ec)
        return;

      std::cout << "TLS handshake" << std::endl;

      // Underlying TLS handshake
      yield amep.next_layer().async_handshake(async_mqtt::tls::stream_base::client, *this);

      std::cout << "TLS handshake ec:" << err.message() << std::endl;

      std::cout << "Connect packet" << std::endl;

      // Send MQTT CONNECT
      yield amep.send(
          async_mqtt::v3_1_1::connect_packet{
              true,                 // clean_session
              0x1234,               // keep_alive
              async_mqtt::allocate_buffer("cid1"),
              async_mqtt::nullopt,  // will
              async_mqtt::nullopt,  // username set like am::allocate_buffer("user1"),
              async_mqtt::nullopt   // password set like am::allocate_buffer("pass1")
          },
          *this);
      if (se) {
        std::cout << "MQTT CONNECT send error:" << se.what() << std::endl;
        return;
      }

      // Recv MQTT CONNACK
      yield amep.recv(*this);
      if (pv) {
        pv.visit(async_mqtt::overload{ [&](async_mqtt::v3_1_1::connack_packet const& p) {
                                        std::cout << "MQTT CONNACK recv"
                                                  << " sp:" << p.session_present() << std::endl;
                                      },
                                       [](auto const&) {} });
      } else {
        std::cout << "MQTT CONNACK recv error:" << pv.get<async_mqtt::system_error>().what() << std::endl;
        return;
      }

      // Send MQTT SUBSCRIBE
      yield amep.send(async_mqtt::v3_1_1::subscribe_packet{ *amep.acquire_unique_packet_id(),
                                                            { { async_mqtt::allocate_buffer("topic1"),
                                                                async_mqtt::qos::at_most_once } } },
                      *this);
      if (se) {
        std::cout << "MQTT SUBSCRIBE send error:" << se.what() << std::endl;
        return;
      }
      // Recv MQTT SUBACK
      yield amep.recv(*this);
      if (pv) {
        pv.visit(async_mqtt::overload{ [&](async_mqtt::v3_1_1::suback_packet const& p) {
                                        std::cout << "MQTT SUBACK recv"
                                                  << " pid:" << p.packet_id() << " entries:";
                                        for (auto const& e : p.entries()) {
                                          std::cout << e << " ";
                                        }
                                        std::cout << std::endl;
                                      },
                                       [](auto const&) {} });
      } else {
        std::cout << "MQTT SUBACK recv error:" << pv.get<async_mqtt::system_error>().what() << std::endl;
        return;
      }
      // Send MQTT PUBLISH
      yield amep.send(
          async_mqtt::v3_1_1::publish_packet{ *amep.acquire_unique_packet_id(), async_mqtt::allocate_buffer("topic1"),
                                              async_mqtt::allocate_buffer("payload1"), async_mqtt::qos::at_least_once },
          *this);
      if (se) {
        std::cout << "MQTT PUBLISH send error:" << se.what() << std::endl;
        return;
      }

      // Recv MQTT PUBLISH and PUBACK (order depends on broker)
      for (count = 0; count != 2; ++count) {
        yield amep.recv(*this);
        if (pv) {
          pv.visit(async_mqtt::overload{ [&](async_mqtt::v3_1_1::publish_packet const& p) {
                                          std::cout << "MQTT PUBLISH recv"
                                                    << " pid:" << p.packet_id() << " topic:" << p.topic()
                                                    << " payload:" << async_mqtt::to_string(p.payload())
                                                    << " qos:" << p.opts().get_qos() << " retain:" << p.opts().get_retain()
                                                    << " dup:" << p.opts().get_dup() << std::endl;
                                        },
                                         [&](async_mqtt::v3_1_1::puback_packet const& p) {
                                           std::cout << "MQTT PUBACK recv"
                                                     << " pid:" << p.packet_id() << std::endl;
                                         },
                                         [](auto const&) {} });
        } else {
          std::cout << "MQTT recv error:" << pv.get<async_mqtt::system_error>().what() << std::endl;
        }
      }
      std::cout << "close" << std::endl;
      yield amep.close(*this);
    }
  }

  asio::ip::tcp::resolver& res;
  std::string host;
  std::string port;
  async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtts>& amep;
  std::size_t count = 0;
  asio::coroutine coro;
};

#include <boost/asio/unyield.hpp>

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " host port" << std::endl;
    return -1;
  }
  async_mqtt::setup_log(async_mqtt::severity_level::trace);
  asio::io_context ioc;
  asio::ip::tcp::socket resolve_sock{ ioc };
  asio::ip::tcp::resolver res{ resolve_sock.get_executor() };
  async_mqtt::tls::context ctx{ async_mqtt::tls::context::tlsv12 };
  ctx.set_verify_mode(async_mqtt::tls::verify_none);
  // If you want to check server certificate, set cacert as follows.
  // signed cert with the private key of the server
  // ctx.load_verify_file(cacert);
  async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtts> amep{ async_mqtt::protocol_version::v3_1_1,
                                                                                    ioc.get_executor(), ctx };
  app a{ res, argv[1], argv[2], amep };
  a();
  ioc.run();
}