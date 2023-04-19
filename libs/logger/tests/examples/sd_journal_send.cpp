
// Small test of sending log messages via boost asio

#include <iostream>
#include <string>
#include <string_view>

#include <boost/asio.hpp>

using namespace std::string_view_literals;
namespace asio = boost::asio;

int main() {
  asio::io_context ctx;
  asio::local::datagram_protocol::socket sock(ctx);

  sock.connect(journald_socket);

  asio::socket_base::send_buffer_size option;
  sock.get_option(option);
  std::cout << "Send socket size: " << option.value() << std::endl;

  std::string test_message = R"(PRIORITY=3
SYSLOG_FACILITY=3
CODE_FILE=src/foobar.c
CODE_LINE=77
CODE_FUNC=some_func
SYSLOG_IDENTIFIER=footool
MESSAGE=Something happened.
)";
  std::cout << test_message << std::endl;
  size_t bytes_sent = sock.send(asio::buffer(test_message));
  std::cout << bytes_sent << std::endl;
}