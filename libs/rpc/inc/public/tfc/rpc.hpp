#include <concepts>
#include <expected>
#include <memory>
#include <mutex>
#include <string_view>

#include <fmt/format.h>
#include <azmq/socket.hpp>
#include <boost/asio/io_context.hpp>
#include <glaze/ext/jsonrpc.hpp>

#include <tfc/logger.hpp>
#include <tfc/progbase.hpp>

namespace concepts {

template <typename jsonrpc_t>
concept jsonrpc_type = requires(jsonrpc_t j_t) {
                         { j_t.call(std::string_view{}) };
                       };

}  // namespace concepts

namespace tfc::rpc {

namespace asio = boost::asio;

enum struct zmq_socket_type_e : std::uint8_t {
  pair = ZMQ_PAIR,
  pub = ZMQ_PUB,
  sub = ZMQ_SUB,
  req = ZMQ_REQ,
  rep = ZMQ_REP,
  dealer = ZMQ_DEALER,
  router = ZMQ_ROUTER,
  pull = ZMQ_PULL,
  push = ZMQ_PUSH,
  xpub = ZMQ_XPUB,
  xsub = ZMQ_XSUB,
  stream = ZMQ_STREAM,
};

enum struct server_error_e : std::underlying_type_t<glz::rpc::error_e> {
  disconnected = std::to_underlying(glz::rpc::error_e::server_error_lower),
  unknown = std::to_underlying(glz::rpc::error_e::server_error_upper),
};

constexpr auto operator==(server_error_e const& lhs, glz::rpc::error_e const& rhs) noexcept -> bool {
  return std::to_underlying(lhs) == std::to_underlying(rhs);
}

template <typename owner_t,
          concepts::jsonrpc_type jsonrpc_server_t,
          zmq_socket_type_e rpc_type,
          bool optimize_single_threaded>
class rpc_skeleton {
public:
  explicit rpc_skeleton(asio::io_context& ctx, std::string_view name)
      : name_{ name }, logger_{ name }, rpc_socket_{ ctx, std::to_underlying(rpc_type), optimize_single_threaded } {
    static_cast<owner_t*>(this)->init();
  }

  rpc_skeleton(rpc_skeleton const&) = delete;
  auto operator=(rpc_skeleton const&) -> rpc_skeleton& = delete;
  rpc_skeleton(rpc_skeleton&&) = delete;
  auto operator=(rpc_skeleton&&) -> rpc_skeleton& = delete;

  [[nodiscard]] auto converter() noexcept -> jsonrpc_server_t& { return jsonrpc_converter_; }
  [[nodiscard]] auto converter() const noexcept -> jsonrpc_server_t const& { return jsonrpc_converter_; }

protected:
  jsonrpc_server_t jsonrpc_converter_{};
  std::string name_{};
  tfc::logger::logger logger_;
  azmq::socket rpc_socket_;
  static constexpr auto qualified_endpoint(std::string_view name) -> std::string { return fmt::format("ipc://{}", name); }
};

template <concepts::jsonrpc_type jsonrpc_server_t, bool optimize_single_threaded = false>
class server : public rpc_skeleton<server<jsonrpc_server_t, optimize_single_threaded>,
                                   jsonrpc_server_t,
                                   zmq_socket_type_e::rep,
                                   optimize_single_threaded> {
public:
  explicit server(asio::io_context& ctx, std::string_view name)
      : rpc_skeleton<server, jsonrpc_server_t, zmq_socket_type_e::rep, optimize_single_threaded>{ ctx, name } {}

  ~server() { this->rpc_socket_.unbind(this->qualified_endpoint(this->name_)); }

  friend class rpc_skeleton<server<jsonrpc_server_t, optimize_single_threaded>,
                            jsonrpc_server_t,
                            zmq_socket_type_e::rep,
                            optimize_single_threaded>;

private:
  auto init() noexcept(false) -> void {
    this->rpc_socket_.bind(this->qualified_endpoint(this->name_));
    this->rpc_socket_.async_receive(
        [this](std::error_code const& err_code, azmq::message const& msg, size_t bytes_transferred) {
          this->receive_handler(err_code, msg, bytes_transferred);
        });
  }
  void receive_handler(std::error_code const& err_code, azmq::message const& msg, size_t bytes_transferred) noexcept {
    if (err_code) {
      this->logger_.warn("Got error: '{}'", err_code.message());
      if (err_code.value() == ECANCELED) {
        // deconstruction might call cancel
        return;
      }
      [[unlikely]] std::terminate();
    }
    try {
      auto const response{ this->jsonrpc_converter_.call(
          std::string_view(static_cast<char const*>(msg.data()), bytes_transferred)) };

      auto const response_shared{ std::make_shared<std::string>(std::move(response)) };
      this->rpc_socket_.async_send(
          azmq::message(*response_shared),
          [this, response_shared](std::error_code const& err, std::size_t socket_bytes_transferred) {
            if (err) {
              this->logger_.warn("Error: '{}' while sending: '{}'. Sent nr of bytes: {}", err.message(), *response_shared,
                                 socket_bytes_transferred);
            }
          });
    } catch (std::exception const& exc) {
      this->logger_.warn("Exception: '{}' will terminate program", exc.what());
      std::terminate();  // maybe instead do io_context stop/restart
    }
    this->rpc_socket_.async_receive(
        [this](std::error_code const& err, azmq::message const& new_msg, size_t socket_bytes_transferred) {
          this->receive_handler(err, new_msg, socket_bytes_transferred);
        });
  }
};

template <concepts::jsonrpc_type jsonrpc_client_t, bool optimize_single_threaded = false>
class client : public rpc_skeleton<client<jsonrpc_client_t, optimize_single_threaded>,
                                   jsonrpc_client_t,
                                   zmq_socket_type_e::req,
                                   optimize_single_threaded> {
public:
  using uuid = std::string;

  explicit client(asio::io_context& ctx, std::string_view name)
      : rpc_skeleton<client, jsonrpc_client_t, zmq_socket_type_e::req, optimize_single_threaded>{ ctx, name },
        uuid_prefix_{ fmt::format("{}.{}.{}", base::get_exe_name(), base::get_proc_name(), this->name_) },
        rpc_socket_monitor_{ this->rpc_socket_.monitor(ctx, ZMQ_EVENT_DISCONNECTED) } {
    rpc_socket_monitor_.async_receive(std::bind(&client::on_disconnect_do_reconnect, this, std::placeholders::_1,
                                                std::placeholders::_2, std::placeholders::_3));
  }

  template <glz::rpc::detail::basic_fixed_string method_name>  // todo how could this be `auto method_name`
  auto async_request(auto&& params, auto&& callback) -> void {
    auto const new_uuid{ make_uuid() };
    auto callback_copy{ callback };
    auto [request_str, inserted] = this->jsonrpc_converter_.template request<method_name>(
        { new_uuid }, std::forward<decltype(params)>(params), std::forward<decltype(callback)>(callback));
    if (!inserted) {
      // should we do asio post instead of firing this synchronously??
      std::invoke(callback_copy,
                  glz::unexpected{
                      glz::rpc::error(glz::rpc::error_e::internal, fmt::format("Unique id: {} is not unique", new_uuid)) },
                  glz::json_t::null_t{});
      return;
    }
    auto request_str_ptr{ std::make_shared<std::string>(std::move(request_str)) };

    auto& ctx{ this->rpc_socket_.get_io_context() };
    auto const on_async_error{ [&ctx, callback_copy](std::error_code const& err) -> void {
      if (err.value() == ECANCELED) {
        asio::post(ctx, [callback_copy]() {
          // Using asio::post so the user can re-request otherwise it would request on the socket which will be destructed
          std::invoke(callback_copy,
                      glz::unexpected{
                          glz::rpc::error{ static_cast<glz::rpc::error_e>(server_error_e::disconnected), "Disconnected" } },
                      glz::json_t::null_t{});
        });
        return;
      }
      std::invoke(
          callback_copy,
          glz::unexpected{ glz::rpc::error(glz::rpc::error_e::internal, fmt::format("Send error: {}", err.message())) },
          glz::json_t::null_t{});
    } };

    this->rpc_socket_.async_send(
        asio::buffer(*request_str_ptr, request_str_ptr->size()),
        [this, request_str_ptr, on_async_error, new_uuid](std::error_code const& send_err_code, std::size_t) {
          if (send_err_code) {
            return on_async_error(send_err_code);
          }
          this->rpc_socket_.async_receive([this, on_async_error, new_uuid](std::error_code const& recv_err_code,
                                                                           azmq::message const& msg,
                                                                           std::size_t bytes_transferred) {
            try {
              if (recv_err_code) {
                return on_async_error(recv_err_code);
              }
              std::string_view const received_msg{ static_cast<char const*>(msg.data()), bytes_transferred };
              auto const rpc_err{ this->jsonrpc_converter_.call(received_msg) };

              if (rpc_err) {
                this->logger_.info("RPC error: '{}' on receive of: '{}'.", rpc_err.get_message(), received_msg);
              }
            } catch (std::exception const& exc) {
              this->logger_.warn("Exception: '{}' will terminate program", exc.what());
              std::terminate();  // maybe instead do io_context stop/restart
            }
          });
        });
  }

  template <glz::rpc::detail::basic_fixed_string method_name>
  auto async_notify(auto&& params) -> void {
    auto request_str_ptr = std::make_shared<std::string>(
        this->jsonrpc_converter_.template notify<method_name>(std::forward<decltype(params)>(params)));

    this->rpc_socket_.async_send(
        asio::buffer(*request_str_ptr, request_str_ptr->size()),
        [request_str_ptr, this](std::error_code const& err, std::size_t) {
          if (err) {
            this->logger_.warn("Error: '{}' during async send of RPC notification", err.message());
            return;
          }
          // We don't really care about the async receive here, we need to acknowledge the empty answer
          this->rpc_socket_.async_receive([](std::error_code const&, azmq::message const&, std::size_t) {});
        });
  }

  friend class rpc_skeleton<client<jsonrpc_client_t, optimize_single_threaded>,
                            jsonrpc_client_t,
                            zmq_socket_type_e::req,
                            optimize_single_threaded>;

private:
  auto init() noexcept(false) -> void { this->rpc_socket_.connect(this->qualified_endpoint(this->name_)); }

  auto reconstruct_rpc_socket() noexcept -> void {
    try {
      this->rpc_socket_.cancel();  // Fire cancel callbacks to user for all requests
      this->rpc_socket_ = azmq::socket{ this->rpc_socket_.get_io_context(), std::to_underlying(zmq_socket_type_e::req),
                                        optimize_single_threaded };
      this->rpc_socket_.connect(this->qualified_endpoint(this->name_));
      rpc_socket_monitor_ = this->rpc_socket_.monitor(this->rpc_socket_.get_io_context(), ZMQ_EVENT_DISCONNECTED);
      rpc_socket_monitor_.async_receive(std::bind(&client::on_disconnect_do_reconnect, this, std::placeholders::_1,
                                                  std::placeholders::_2, std::placeholders::_3));
    } catch (std::exception const& exc) {
      this->logger_.warn("Got exception: '{}' will terminate program", exc.what());
      std::terminate();
    }
  }

  auto make_uuid() -> uuid {
    std::unique_lock<std::mutex> scope_lock{ mtx_ };
    return fmt::format("{}.{}", uuid_prefix_, ++cnt_);
  }

  void on_disconnect_do_reconnect(std::error_code const& err_code, azmq::message const&, size_t) noexcept {
    if (err_code) {
      assert(false && "Handle event disconnected got error");
      return;
    }
    this->logger_.info("RPC socket got disconnected");
    // We don't really care what happens with the next receive
    // Since the monitoring receive sends two packets for each event
    boost::system::error_code err{};
    std::array<std::byte, 1024> buffer{};
    rpc_socket_monitor_.receive(asio::buffer(buffer), 0, err);

    // Down to business, now we will cancel all queued request and the user will receive disconnected server error.
    reconstruct_rpc_socket();
  }

  std::uint64_t cnt_{};
  const std::string uuid_prefix_{};
  std::mutex mtx_{};
  azmq::socket rpc_socket_monitor_;
};

}  // namespace tfc::rpc
