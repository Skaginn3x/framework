#include <boost/beast.hpp>
#include <boost/program_options.hpp>

#include <tfc/ipc.hpp>
#include <tfc/progbase.hpp>

#include <algorithm>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace beast = boost::beast;          // from <boost/beast.hpp>
namespace http = beast::http;            // from <boost/beast/http.hpp>
namespace websocket = beast::websocket;  // from <boost/beast/websocket.hpp>
namespace net = boost::asio;             // from <boost/asio.hpp>
namespace bpo = boost::program_options;
using tcp = boost::asio::ip::tcp;        // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// Report a failure
static void fail(beast::error_code error_code, char const* what) {
  std::cerr << what << ": " << error_code.message() << "\n";
}

// Echoes back all received WebSocket messages
class session : public std::enable_shared_from_this<session> {
  websocket::stream<beast::tcp_stream> ws_;
  beast::flat_buffer buffer_;
  net::io_context& ctx_;
  tfc::ipc::any_recv_cb recv_;
  char ping_state_ = 0;
  boost::asio::steady_timer timer_;

public:
  ~session() { std::cerr << "Session dtor" << std::endl; }
  // Take ownership of the socket
  explicit session(tcp::socket&& socket, net::io_context& ctx)
      : ws_(std::move(socket)), ctx_(ctx), timer_(ctx_, std::chrono::steady_clock::time_point::max()) {}

  // Get on the correct executor
  void run() {
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.
    net::dispatch(ws_.get_executor(), beast::bind_front_handler(&session::on_run, shared_from_this()));
  }

  // Start the asynchronous operation
  void on_run() {
    // Set suggested timeout settings for the websocket
    ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    ws_.set_option(websocket::stream_base::decorator([](websocket::response_type& res) {
      res.set(http::field::server, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-server-async");
    }));
    // Accept the websocket handshake
    ws_.async_accept(beast::bind_front_handler(&session::on_accept, shared_from_this()));
  }

  void on_accept(beast::error_code error_code) {
    if (error_code) {
      return fail(error_code, "accept");
    }
    ws_.control_callback(
        [this](auto && PH1, auto && PH2) { on_control_callback(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });

    on_timer({});
    timer_.expires_after(std::chrono::seconds(15));

    recv_ =
        tfc::ipc::create_ipc_recv_cb<tfc::ipc::any_recv_cb>(ctx_, "ec_example_run_context.test.atv320.string.command.slot");
    std::visit(
        [&](auto&& receiver) {
          using receiver_t = std::remove_cvref_t<decltype(receiver)>;
          if constexpr (!std::same_as<std::monostate, receiver_t>) {
            using value_t = typename std::remove_cvref_t<decltype(receiver)>::element_type::value_t;
            auto error =
                receiver->init("ec_example_run_context.test.atv320.string.command",
                               [weak_self = weak_from_this()](std::expected<value_t, std::error_code> value) {
                                 auto self = weak_self.lock();
                                 if (self) {
                                   if (!value.has_value()) {
                                     throw std::runtime_error(value.error().message());
                                   }
                                   self->ws_.text(true);
                                   auto to_send = std::make_shared<std::string>(fmt::format("{}", value.value()));
                                   self->ws_.async_write(net::buffer(*to_send), [to_send](beast::error_code, std::size_t) {

                                   });
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
  void on_ping(boost::system::error_code error_code) {
    // Happens when the timer closes the socket
    if (error_code == boost::asio::error::operation_aborted) {
      return;
    }

    if (error_code) {
      return fail(error_code, "ping");
    }

    // Note that the ping was sent.
    if (ping_state_ == 1) {
      ping_state_ = 2;
    } else {
      // ping_state_ could have been set to 0
      // if an incoming control frame was received
      // at exactly the same time we sent a ping.
      BOOST_ASSERT(ping_state_ == 0);
    }
  }

  void on_control_callback(websocket::frame_type kind, boost::beast::string_view payload) {
    boost::ignore_unused(kind, payload);

    std::cout << "recived control event" << std::endl;
    // Note that there is activity
    on_timer({});
  }
  // Called when the timer expires.
  void on_timer(boost::system::error_code error_code) {
    if (error_code && error_code != boost::asio::error::operation_aborted) {
      return fail(error_code, "timer");
    }

    // See if the timer really expired since the deadline may have moved.
    if (timer_.expiry() <= std::chrono::steady_clock::now()) {
      // If this is the first time the timer expired,
      // send a ping to see if the other end is there.
      if (ws_.is_open() && ping_state_ == 0) {
        // Note that we are sending a ping
        ping_state_ = 1;

        // Set the timer
        timer_.expires_after(std::chrono::seconds(15));

        // Now send the ping
        std::cout << "Sending ping" << std::endl;
        ws_.async_ping(
            {}, boost::asio::bind_executor(ctx_, [capture0 = shared_from_this()](auto && PH1) { capture0->on_ping(std::forward<decltype(PH1)>(PH1)); }));
      } else {
        // The timer expired while trying to handshake,
        // or we sent a ping and it never completed or
        // we never got back a control frame, so close.

        // Closing the socket cancels all outstanding operations. They
        // will complete with boost::asio::error::operation_aborted
        ws_.next_layer().socket().shutdown(tcp::socket::shutdown_both, error_code);
        ws_.next_layer().socket().close(error_code);
        std::cerr << "Closing" << std::endl;
        return;
      }
    }

    // Wait on the timer
    timer_.async_wait(
        boost::asio::bind_executor(ctx_, [capture0 = shared_from_this()](auto && PH1) { capture0->on_timer(std::forward<decltype(PH1)>(PH1)); }));
  }
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener> {
  net::io_context& ioc_;
  tcp::acceptor acceptor_;

public:
  listener(net::io_context& ioc, const tcp::endpoint& endpoint) : ioc_(ioc), acceptor_(ioc) {
    beast::error_code error_code;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), error_code);
    if (error_code) {
      fail(error_code, "open");
      return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true), error_code);
    if (error_code) {
      fail(error_code, "set_option");
      return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, error_code);
    if (error_code) {
      fail(error_code, "bind");
      return;
    }

    // Start listening for connections
    acceptor_.listen(net::socket_base::max_listen_connections, error_code);
    if (error_code) {
      fail(error_code, "listen");
      return;
    }
  }

  // Start accepting incoming connections
  void run() { do_accept(); }

private:
  void do_accept() {
    // The new connection gets its own strand
    acceptor_.async_accept(net::make_strand(ioc_), beast::bind_front_handler(&listener::on_accept, shared_from_this()));
  }

  void on_accept(beast::error_code error_code, tcp::socket socket) {
    if (error_code) {
      fail(error_code, "accept");
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

  auto const address = net::ip::make_address(addr_str);

  // The io_context is required for all I/O
  net::io_context ioc{};

  // Create and launch a listening port
  std::make_shared<listener>(ioc, tcp::endpoint{ address, port })->run();

  // Run the I/O service on the requested number of threads
  ioc.run();

  return EXIT_SUCCESS;
}
