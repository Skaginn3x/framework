#include <cstdlib>
#include <string>
#include <vector>

#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/program_options.hpp>
#include <boost/url/src.hpp>

#include <tfc/ipc.hpp>
#include <tfc/logger.hpp>
#include <tfc/progbase.hpp>

namespace beast = boost::beast;          // from <boost/beast.hpp>
namespace http = beast::http;            // from <boost/beast/http.hpp>
namespace websocket = beast::websocket;  // from <boost/beast/websocket.hpp>
namespace asio = boost::asio;            // from <boost/asio.hpp>
namespace bpo = boost::program_options;
using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

enum struct ping_state : uint8_t {
  not_sent_e = 0,
  sent_e = 1,
  waiting_e = 2,
};

class session : public std::enable_shared_from_this<session> {
public:
  ~session() { std::cerr << "Session dtor"; }

  // Take ownership of the socket
  explicit session(tcp::socket&& socket, asio::io_context& ctx)
      : ws_(std::move(socket)), ctx_(ctx), timer_(ctx_, std::chrono::steady_clock::time_point::max()),
        logger_(fmt::format("session-{}:{}",
                            ws_.next_layer().socket().remote_endpoint().address().to_string(),
                            ws_.next_layer().socket().remote_endpoint().port())) {}

  // Get on the correct executor
  void run() { asio::dispatch(ws_.get_executor(), std::bind_front(&session::on_run, shared_from_this())); }

  // Start the asynchronous operation
  void on_run() {
    // Disable internal ping sending and idle monitoring. first parameter here is for handshake timeout.
    // The underlying ipc socket azmq hangs if there is an outstanding read.
    // We therefor keep a shared_ptr reference inside the ping-pong sequence. That socket
    // Calls with an error on socket close. Then the read operation on azmq has a weak_ptr reference
    // Causing that socket to deconstruct if the ping-pong operation cancels. That is why the internal
    // mechanism is not utilized.
    ws_.set_option(websocket::stream_base::timeout{ std::chrono::seconds(10), websocket::stream_base::none(), false });

    // Recieve the upgrade request as a normal http request to parse out
    // the uri parameters rather than use beasts internal accept mechanism
    http::request<http::string_body> req;
    boost::beast::flat_buffer buffer;
    http::read(ws_.next_layer(), buffer, req);
    if (websocket::is_upgrade(req)) {
      try {
        std::string protocol = req[http::field::sec_websocket_protocol];

        logger_.trace("URI '{}'", req.base().target());
        auto const url_params = boost::urls::url_view(req.base().target()).params();
        auto const connect = url_params.find("connect");
        if (protocol.empty() || protocol != "tfc-ipc" || connect == url_params.end() || (*connect).has_value ||
            (*connect).value.empty()) {
          logger_.warn("Refusing connection");
          http::response<http::string_body> res;
          res.result(http::status::bad_request);
          res.body() = "Either a valid protocol is missing or a parameter for that protocol is missing";
          http::write(ws_.next_layer(), res);
        } else {
          logger_.trace("Accepting connection for tfc-ipc protocol");
          // Set the websocket protocol in the response, chrome requires this.
          ws_.set_option(websocket::stream_base::decorator([](http::response<http::string_body>& res) {
            res.set(http::field::sec_websocket_protocol, "tfc-ipc");
            res.set(http::field::server, "ws-bridge");
          }));
          std::string connect_value = (*connect).value;
          ws_.async_accept(req, std::bind_front(&session::on_accept_ipc_slot, shared_from_this(), connect_value));
        }

      } catch (std::exception const& err) {
        logger_.error("error - {}", err.what());
      }
    }
  }

  void handle_read(boost::system::error_code const& error_code, size_t const bytes_received) {
    if (error_code && error_code != boost::asio::error::eof) {
      logger_.trace("handle_read error - {}", error_code.message());
      return;
    }
    logger_.info("Client sent {} of size {} to a write only websocket!", beast::buffers_to_string(buffer_.data()),
                 bytes_received);

    // Clear the buffer again
    buffer_.consume(bytes_received);

    // retrigger the read
    ws_.async_read_some(buffer_, 150, [weak_self = weak_from_this()](std::error_code const& err, size_t const count) {
      auto self = weak_self.lock();
      if (self) {
        self->handle_read(err, count);
      }
    });
  }

  void on_accept_ipc_slot(std::string const slot_name, beast::error_code const& error_code) {
    if (error_code) {
      logger_.error("accept - {}", error_code.message());
      return;
    }
    ws_.text(true);
    ws_.control_callback([weak_self = weak_from_this()](auto&& PH1, auto&& PH2) {
      auto self = weak_self.lock();
      if (self) {
        self->on_control_callback(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
      }
    });

    // Always have a read in the background to recieve control events
    ws_.async_read_some(buffer_, 150, [weak_self = weak_from_this()](std::error_code const& err, size_t bytes_received) {
      auto self = weak_self.lock();
      if (self) {
        self->handle_read(err, bytes_received);
      }
    });

    on_timer({});
    timer_.expires_after(std::chrono::seconds(15));

    recv_ = tfc::ipc::create_ipc_recv_cb<tfc::ipc::any_recv_cb>(ctx_, fmt::format("{}.slot", slot_name));
    std::visit(
        [&](auto&& receiver) {
          using receiver_t = std::remove_cvref_t<decltype(receiver)>;
          if constexpr (!std::same_as<std::monostate, receiver_t>) {
            using value_t = typename std::remove_cvref_t<decltype(receiver)>::element_type::value_t;
            auto error =
                receiver->init(slot_name, [weak_self = weak_from_this()](std::expected<value_t, std::error_code> value) {
                  auto self = weak_self.lock();
                  if (self) {
                    if (!value.has_value()) {
                      throw std::runtime_error(value.error().message());
                    }
                    auto to_send = fmt::format("{}", value.value());
                    // Boost beast only supports a single async_write operation
                    // in flight at any given moment. Send this sync for now.
                    try {
                      size_t bytes_transfered = self->ws_.write(asio::buffer(to_send));
                      if (bytes_transfered < to_send.size()) {
                        self->logger_.error("Wrote less than to send");
                      }
                    } catch (std::exception const& err) {
                      self->logger_.error("Write - {}", err.what());
                      // Drop the connection by setting the timeout to current time and then canceling the timer.
                      // And setting ping state to 2
                      self->ping_state_ = ping_state::waiting_e;
                      self->timer_.expires_after(std::chrono::seconds(0));
                      self->timer_.cancel();
                    }
                  }
                });
            if (error) {
              throw std::runtime_error(error.message());
            }
          }
        },
        recv_);
  }

  // Called after a ping is sent.
  void on_ping(boost::system::error_code const& error_code) {
    // Happens when the timer closes the socket
    if (error_code == boost::asio::error::operation_aborted) {
      return;
    }

    if (error_code) {
      logger_.error("ping - {}", error_code.message());
      return;
    }

    // Note that the ping was sent.
    if (ping_state_ == ping_state::sent_e) {
      ping_state_ = ping_state::waiting_e;
    } else {
      // ping_state_ could have been set to 0
      // if an incoming control frame was received
      // at exactly the same time we sent a ping.
      BOOST_ASSERT(ping_state_ == ping_state::not_sent_e);
    }
  }

  void on_control_callback(websocket::frame_type kind, boost::beast::string_view payload) {
    logger_.trace("received control event {}", payload);
    // Note there is activity
    ping_state_ = ping_state::not_sent_e;
    if (kind == websocket::frame_type::pong) {
      on_timer({});
    }
  }

  // Called when the timer expires.
  void on_timer(boost::system::error_code error_code) {
    if (error_code && error_code != boost::asio::error::operation_aborted) {
      logger_.error("timer - {}", error_code.message());
      return;
    }

    // See if the timer really expired since the deadline may have moved.
    if (timer_.expiry() <= std::chrono::steady_clock::now()) {
      // If this is the first time the timer expired,
      // send a ping to see if the other end is there.
      if (ws_.is_open() && ping_state_ == ping_state::not_sent_e) {
        // Note that we are sending a ping
        ping_state_ = ping_state::sent_e;

        // Set the timer
        timer_.expires_after(std::chrono::seconds(15));

        // Now send the ping
        logger_.trace("Sending ping");
        ws_.async_ping({},
                       boost::asio::bind_executor(
                           ctx_, [self = shared_from_this()](boost::system::error_code const& err) { self->on_ping(err); }));
      } else {
        // The timer expired while trying to handshake,
        // or we sent a ping and it never completed or
        // we never got back a control frame, so close.

        // Closing the socket cancels all outstanding operations. They
        // will complete with boost::asio::error::operation_aborted
        logger_.warn("Closing");
        ws_.next_layer().socket().shutdown(tcp::socket::shutdown_both, error_code);
        ws_.next_layer().socket().close(error_code);

        // Cancel the timer, the session is closed now
        timer_.cancel();
        return;
      }
    }

    // Wait on the timer
    timer_.async_wait(boost::asio::bind_executor(
        ctx_, [capture0 = shared_from_this()](auto&& PH1) { capture0->on_timer(std::forward<decltype(PH1)>(PH1)); }));
  }

private:
  websocket::stream<beast::tcp_stream> ws_;
  beast::flat_buffer buffer_;
  asio::io_context& ctx_;
  tfc::ipc::any_recv_cb recv_;
  ping_state ping_state_ = ping_state::not_sent_e;
  boost::asio::steady_timer timer_;
  tfc::logger::logger logger_;
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener> {
  asio::io_context& ioc_;
  tcp::acceptor acceptor_;
  tfc::logger::logger logger_;

public:
  listener(asio::io_context& ioc, const tcp::endpoint& endpoint) : ioc_(ioc), acceptor_(ioc), logger_("listener") {
    beast::error_code error_code;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), error_code);
    if (error_code) {
      logger_.error("open - {}", error_code.message());
      return;
    }

    // Allow address reuse
    acceptor_.set_option(asio::socket_base::reuse_address(true), error_code);
    if (error_code) {
      logger_.error("set_option - {}", error_code.message());
      return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, error_code);
    if (error_code) {
      logger_.error("bind - {}", error_code.message());
      return;
    }

    // Start listening for connections
    acceptor_.listen(asio::socket_base::max_listen_connections, error_code);
    if (error_code) {
      logger_.error("listen - {}", error_code.message());
      return;
    }
  }

  // Start accepting incoming connections
  void run() { do_accept(); }

private:
  void do_accept() {
    // The new connection gets its own strand
    acceptor_.async_accept(asio::make_strand(ioc_), std::bind_front(&listener::on_accept, shared_from_this()));
  }

  void on_accept(beast::error_code error_code, tcp::socket socket) {
    if (error_code) {
      logger_.error("accept - {}", error_code.message());
    } else {
      // Create the session and run it
      std::make_shared<session>(std::move(socket), ioc_)->run();
    }

    // Accept another connection
    do_accept();
  }
};

//------------------------------------------------------------------------------

auto main(int argc, char* argv[]) -> int {
  auto desc{ tfc::base::default_description() };
  std::string addr_str;
  uint16_t port;
  desc.add_options()("addr,a", bpo::value<std::string>(&addr_str)->required())("port,p",
                                                                               bpo::value<uint16_t>(&port)->required());
  tfc::base::init(argc, argv, desc);

  auto const address = asio::ip::make_address(addr_str);

  // The io_context is required for all I/O
  asio::io_context ioc{};

  // Create and launch a listening port
  std::make_shared<listener>(ioc, tcp::endpoint{ address, port })->run();

  // Run the I/O service on the requested number of threads
  ioc.run();

  return EXIT_SUCCESS;
}
