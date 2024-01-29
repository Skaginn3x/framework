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
#include <constants.hpp>

#include <tfc/progbase.hpp>

#include <run.hpp>

#include <boost/ut.hpp>
namespace ut = boost::ut;
using ut::operator""_test;
using ut::expect;

namespace asio = boost::asio;
// namespace pr = boost::process;
namespace am = async_mqtt;

struct message {
  std::string topic;
  std::string payload;
};

class mqtt_broker {
public:
  mqtt_broker(asio::io_context& io_ctx)
      : io_ctx_(io_ctx), mqtt_endpoint_(asio::ip::tcp::v4(), 1883), mqtt_acceptor_(io_ctx_, mqtt_endpoint_),
        broker_(io_ctx_) {
    setupAsyncAccept();
  }

private:
  void setupAsyncAccept() {
    mqtt_async_accept_ = [this] {
      auto endpoint = am::endpoint<am::role::server, am::protocol::mqtt>::create(am::protocol_version::undetermined,
                                                                                 io_ctx_.get_executor());

      auto& lowest_layer = endpoint->lowest_layer();
      mqtt_acceptor_.async_accept(lowest_layer, [this, endpoint](boost::system::error_code const& ec) mutable {
        if (!ec) {
          broker_.handle_accept(epv_t{ std::move(endpoint) });
          mqtt_async_accept_();
        } else {
          std::cerr << "TCP accept error: " << ec.message() << std::endl;
        }
      });
    };

    mqtt_async_accept_();
  }

  asio::io_context& io_ctx_;
  asio::ip::tcp::endpoint mqtt_endpoint_;
  asio::ip::tcp::acceptor mqtt_acceptor_;
  std::function<void()> mqtt_async_accept_;
  using epv_t = am::endpoint_variant<am::role::server, am::protocol::mqtt>;
  am::broker<epv_t> broker_;
};

class mqtt_client {
public:
  mqtt_client(asio::io_context& io_ctx, std::vector<message>& messages, std::string& topic)
      : io_ctx_(io_ctx), messages_(messages),
        amep_(am::endpoint<am::role::client, am::protocol::mqtt>::create(am::protocol_version::v5, io_ctx.get_executor())),
        resolver_(io_ctx), topic_(topic) {
    connect("127.0.0.1", "1883");
  }

  void connect(const std::string& host, const std::string& port) {
    resolver_.async_resolve(host, port, [&](boost::system::error_code ec, asio::ip::tcp::resolver::results_type eps) {
      if (ec)
        return;
      handle_resolve(eps);
    });
  }

private:
  void handle_resolve(asio::ip::tcp::resolver::results_type eps) {
    asio::async_connect(amep_->next_layer(), eps, [&](boost::system::error_code, asio::ip::tcp::endpoint /*unused*/) {
      amep_->send(
          am::v5::connect_packet{
              true,
              0x1234,
              am::allocate_buffer("cid2"),
              am::nullopt,
              am::nullopt,
              am::nullopt,
          },
          [&](am::system_error const&) {
            amep_->recv([&](am::packet_variant pv) {
              pv.visit(am::overload{
                  [&](am::v5::connack_packet const&) {
                    amep_->send(
                        am::v5::subscribe_packet{ *amep_->acquire_unique_packet_id(),
                                                  { { am::allocate_buffer(topic_), am::qos::at_most_once } } },
                        [&](am::system_error const&) {
                          amep_->recv([&](am::packet_variant pv) {
                            pv.visit(am::overload{ [&](am::v5::suback_packet const&) {
                                                    auto recv_handler =
                                                        std::make_shared<std::function<void(am::packet_variant pv)>>();
                                                    *recv_handler = [&, recv_handler](am::packet_variant pv) {
                                                      pv.visit(am::overload{ [&](am::v5::publish_packet const& p) {
                                                                              messages_.emplace_back(p.topic().data(),
                                                                                                     p.payload()[0].data());
                                                                            },
                                                                             [](auto const&) {} });
                                                    };
                                                    amep_->recv(*recv_handler);
                                                  },
                                                   [](auto const&) {} });
                          });
                        });
                  },
                  [](auto const&) {} });
            });
          });
    });
  }

  asio::io_context& io_ctx_;
  std::vector<message>& messages_;
  decltype(am::endpoint<am::role::client, am::protocol::mqtt>::create(am::protocol_version::v5)) amep_;
  asio::ip::tcp::resolver resolver_;
  std::string& topic_;
};

auto main(int argc, char* argv[]) -> int {
  tfc::base::init(argc, argv);

  asio::io_context io_ctx{};

  std::vector<message> messages;

  mqtt_broker broker{ io_ctx };

  io_ctx.run_for(std::chrono::seconds{ 1 });

  std::string nbirth_topic = "spBv1.0/tfc_unconfigured_group_id/NBIRTH/tfc_unconfigured_node_id";

  mqtt_client cli{ io_ctx, messages, nbirth_topic };

  io_ctx.run_for(std::chrono::seconds{ 1 });

  tfc::mqtt::run running{ io_ctx };

  asio::co_spawn(io_ctx, running.start(), asio::detached);

  io_ctx.run_for(std::chrono::seconds{ 1 });

  ut::expect(messages.size() == 1);

  if (messages.size() >= 1) {
    ut::expect(messages[0].topic == nbirth_topic);
    // ut::expect(messages[0].payload == "1");
  }

  return 0;
}
