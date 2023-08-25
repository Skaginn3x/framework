#pragma once

#include <array>
#include <concepts>
#include <expected>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <variant>

#include <fmt/format.h>
#include <azmq/socket.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/system/error_code.hpp>

#include <tfc/ipc/enums.hpp>
#include <tfc/ipc/packet.hpp>
#include <tfc/progbase.hpp>
#include <tfc/utils/pragmas.hpp>
#include <tfc/utils/socket.hpp>

namespace tfc::ipc::details {

namespace asio = boost::asio;

enum struct ipc_errors_e {
  message_to_small = 1,
  inconsistent_size = 2,
};

namespace concepts {
template <typename given_t, typename... supposed_t>
concept is_any_of = (std::same_as<given_t, supposed_t> || ...);
template <typename given_t>
concept is_supported_type = is_any_of<given_t, bool, std::int64_t, std::uint64_t, std::double_t, std::string>;
}  // namespace concepts

template <concepts::is_supported_type value_type, type_e type_enum>
struct type_description {
  using value_t = value_type;
  static constexpr auto value_e = type_enum;
  static constexpr std::string_view type_name{ type_e_iterable[std::to_underlying(type_enum)] };
};

/**@brief
 * Base class for signal and slot. Contains naming
 * and shared ptr factory constructors.
 * */
template <typename type_desc>
class transmission_base {
public:
  explicit transmission_base(std::string_view name) : name_(name) {}

  [[nodiscard]] auto endpoint() const -> std::string { return utils::socket::zmq::ipc_endpoint_str(name_w_type()); }

  /// \return <type>.<name>
  [[nodiscard]] auto name_w_type() const -> std::string {
    return fmt::format("{}.{}.{}.{}", base::get_exe_name(), base::get_proc_name(), type_desc::type_name, name_);
  }

  static constexpr std::string_view path_prefix{ "/tmp/" };  // todo remove

private:
  std::string name_;  // name of signal/slot
};

/**@brief
 * Signal publishing class.
 * */
template <typename type_desc>
class signal : public transmission_base<type_desc>, public std::enable_shared_from_this<signal<type_desc>> {
public:
  using value_t = typename type_desc::value_t;
  using packet_t = packet<value_t, type_desc::value_e>;
  static auto constexpr direction_v = direction_e::signal;

  [[nodiscard]] static auto create(asio::io_context& ctx, std::string_view name)
      -> std::expected<std::shared_ptr<signal<type_desc>>, std::error_code> {
    auto ptr = std::shared_ptr<signal<type_desc>>(new signal(ctx, name));
    auto error = ptr->init();
    if (error) {
      return std::unexpected(error);
    }
    return ptr;
  }
  /// @brief send value to subscriber
  /// @param value is sent
  /// @return std::error_code, empty if no error.
  auto send(value_t const& value) -> std::error_code {
    last_value_ = value;
    std::vector<std::byte> send_buffer{};
    if (auto serialize_err{ packet_t::serialize(last_value_, send_buffer) }) {
      return serialize_err;
    }
    std::size_t size = socket_.send(asio::buffer(send_buffer));
    if (size != send_buffer.size()) {
      return std::make_error_code(std::errc::value_too_large);
    }
    return {};
  }

  /// @brief send value to subscriber
  /// @tparam completion_token_t a concept of type void(std::error_code, std::size_t)
  /// @param value is sent
  template <typename completion_token_t>
  auto async_send(value_t const& value, completion_token_t&& token)
      -> asio::async_result<std::decay_t<completion_token_t>, void(std::error_code, std::size_t)>::return_type {
    last_value_ = value;
    std::unique_ptr<std::vector<std::byte>> send_buffer{ std::make_unique<std::vector<std::byte>>() };
    if (auto serialize_error{ packet_t::serialize(last_value_, *send_buffer) }) {
      return asio::async_compose<completion_token_t, void(std::error_code, std::size_t)>(
          [serialize_error](auto& self, std::error_code = {}, std::size_t = 0) { self.complete(serialize_error, 0); },
          token);
    }

    enum struct state_e { write, complete };

    auto& socket{ socket_ };
    return asio::async_compose<completion_token_t, void(std::error_code, std::size_t)>(
        [&socket, buffer = std::move(send_buffer), state = state_e::write](auto& self, std::error_code err = {},
                                                                           std::size_t bytes_sent = 0) mutable {
          if (err) {
            self.complete(err, bytes_sent);
            return;
          }
          switch (state) {
            case state_e::write: {
              state = state_e::complete;
              azmq::async_send(socket, asio::buffer(*buffer), std::move(self));
              break;
            }
            case state_e::complete: {
              self.complete(err, bytes_sent);
              break;
            }
          }
        },
        token, socket_);
  }

private:
  signal(asio::io_context& ctx, std::string_view name)
      : transmission_base<type_desc>(name), last_value_(), timer_(ctx), socket_(ctx),
        socket_monitor_(socket_.monitor(ctx, ZMQ_EVENT_HANDSHAKE_SUCCEEDED)) {}

  auto init() -> std::error_code {
    boost::system::error_code error_code;
    socket_.bind(this->endpoint(), error_code);
    if (error_code) {
      return error_code;
    }
    register_handle_accept();
    return {};
  }

  void handle_event_accept(std::error_code const& error_code, azmq::message&, size_t) {
    if (error_code) {
      assert(false && "Handle event accept canceled!");
      return;
    }
    std::array<std::byte, 1024> buffer;
    socket_monitor_.receive(asio::buffer(buffer), 0);
    // TODO: This sleep is the worst.
    timer_.expires_after(std::chrono::milliseconds(1));
    timer_.async_wait([this](std::error_code const& error) {
      if (error) {
        return;
      }
      async_send(last_value_, [&](std::error_code err, size_t) {
        if (err) {
          assert(false && "Handle event accept (send) canceled!");
          return;
        }
        register_handle_accept();
      });
    });
  }

  void register_handle_accept() {
    auto bind_reference = std::enable_shared_from_this<signal<type_desc>>::weak_from_this();
    socket_monitor_.async_receive(
        [bind_reference](std::error_code const& error_code, azmq::message& msg, size_t bytes_received) {
          if (auto instance = bind_reference.lock()) {
            instance->handle_event_accept(error_code, msg, bytes_received);
          }
        });
  }
  value_t last_value_{};
  boost::asio::steady_timer timer_;
  azmq::pub_socket socket_;
  azmq::socket socket_monitor_;
};

/**@brief slot
 *
 * */
template <typename type_desc>
class slot : public transmission_base<type_desc> {
public:
  using value_t = typename type_desc::value_t;
  static auto constexpr value_e{ type_desc::value_e };
  using packet_t = packet<value_t, value_e>;
  static auto constexpr direction_v = direction_e::slot;

  [[nodiscard]] static auto create(asio::io_context& ctx, std::string_view name) -> std::shared_ptr<slot<type_desc>> {
    return std::shared_ptr<slot<type_desc>>(new slot(ctx, name));
  }
  slot(asio::io_context& ctx, std::string_view name) : transmission_base<type_desc>(name), socket_(ctx) {}
  /**
   * @brief
   * connect to the signal indicated by name
   * */
  auto connect(std::string_view signal_name) -> std::error_code {
    // TODO: Find out if these mutexes inside optimize single threaded are really needed
    socket_ = azmq::sub_socket(socket_.get_io_context(), true);
    boost::system::error_code error_code;
    std::string const socket_path{ utils::socket::zmq::ipc_endpoint_str(signal_name) };
    if (socket_.connect(socket_path, error_code)) {
      return error_code;
    }
    if (socket_.set_option(azmq::socket::subscribe(""), error_code)) {
      return error_code;
    }
    return {};
  }

  /**
   * @brief synchronous reception of slot data.
   * @return a new value sent to the slot
   */
  [[nodiscard]] auto receive() -> std::expected<value_t&, std::error_code> {
    std::array<std::byte, 1024> buffer;
    boost::system::error_code code;
    auto bytes_received = socket_.receive(buffer, 0, code);

    if (bytes_received < packet_t::header_size()) {
      return std::unexpected(ipc_errors_e::message_to_small);
    }

    return packet_t::deserialize(std::span(buffer.data(), bytes_received)).value;
  }

  /// \brief schedule an async_read on the slot
  /// \tparam completion_token_t completion token in asio format, example a callback or coroutine handler
  /// callback of format: void(std::expected<type_desc::value_t, std::error_code>)
  /// coroutine either asio::awaitable<std::expected<value_t, std::error_code>> or
  /// asio::experimental::coro<void, std::expected<value_t, std::error_code>>
  template <typename completion_token_t>
  auto async_receive(completion_token_t&& token)
      -> asio::async_result<std::decay_t<completion_token_t>, void(std::expected<value_t, std::error_code>)>::return_type {
    enum struct state_e { read, complete };

    std::unique_ptr<std::vector<std::byte>> receive_buffer{ std::make_unique<std::vector<std::byte>>() };
    receive_buffer->resize(4096, {});
    // todo receive header first then value

    azmq::sub_socket& socket{ socket_ };
    return asio::async_compose<completion_token_t, void(std::expected<value_t, std::error_code>)>(
        [&socket, state = state_e::read, buffer = std::move(receive_buffer)](auto& self, std::error_code err = {},
                                                                             std::size_t bytes_received = 0) mutable {
          if (err) {
            self.complete(std::unexpected(err));
            return;
          }
          switch (state) {
            case state_e::read: {
              state = state_e::complete;
              azmq::async_receive(socket, asio::buffer(*buffer), std::move(self));
              break;
            }
            case state_e::complete: {
              self.complete(packet_t::deserialize(std::span{ buffer->data(), bytes_received }));
              break;
            }
          }
        },
        token, socket_);
  }

  /**
   * @brief disconnect from signal
   */
  auto disconnect(std::string_view signal_name) {
    [[maybe_unused]] boost::system::error_code code;
    return socket_.disconnect(signal_name.data(), code);
  }

private:
  azmq::sub_socket socket_;
};

template <typename type_desc>
class slot_callback : public std::enable_shared_from_this<slot_callback<type_desc>> {
public:
  using value_t = type_desc::value_t;
  static auto constexpr direction_v = slot<type_desc>::direction_v;

  [[nodiscard]] static auto create(asio::io_context& ctx, std::string_view name)
      -> std::shared_ptr<slot_callback<type_desc>> {
    return std::shared_ptr<slot_callback<type_desc>>(new slot_callback(ctx, name));
  }

  auto init(
      std::string_view signal_name,
      std::function<void(value_t const&)> callback = [](value_t const&) {}) -> std::error_code {
    if (auto error = slot_.connect(signal_name)) {
      return error;
    }
    cb_ = callback;
    register_read();
    return {};
  }
  [[nodiscard]] auto get() const noexcept -> std::optional<value_t> const& { return last_value_; }

  /**
   * @brief disconnect from signal
   */
  auto disconnect(std::string_view signal_name) { return slot_.disconnect(signal_name.data()); }

  /// \return <type>.<name> for example: bool.my_name
  [[nodiscard]] auto name_w_type() const -> std::string { return slot_.name_w_type(); }

private:
  slot_callback(asio::io_context& ctx, std::string_view name) : slot_(ctx, name) {}
  void async_new_state(std::expected<value_t, std::error_code> value) {
    if (!value) {
      return;
    }
    // Don't retransmit transmitted things.
    // clang-format off
    PRAGMA_CLANG_WARNING_PUSH_OFF(-Wfloat-equal)
    if (!last_value_.has_value() || value.value() != last_value_) {
    PRAGMA_CLANG_WARNING_POP
      // clang-format on
      last_value_ = value.value();
      cb_(last_value_.value());
    }
    register_read();
  }
  void register_read() {
    auto bind_reference = std::enable_shared_from_this<slot_callback<type_desc>>::weak_from_this();
    slot_.async_receive([bind_reference](std::expected<value_t, std::error_code> value) {
      if (auto sptr = bind_reference.lock()) {
        sptr->async_new_state(value);
      }
    });
  }
  slot<type_desc> slot_;
  std::function<void(value_t const&)> cb_{ [](value_t const&) {} };
  std::optional<value_t> last_value_{};
};

using type_bool = type_description<bool, type_e::_bool>;
using type_int = type_description<std::int64_t, type_e::_int64_t>;
using type_uint = type_description<std::uint64_t, type_e::_uint64_t>;
using type_double = type_description<std::double_t, type_e::_double_t>;
using type_string = type_description<std::string, type_e::_string>;
using type_json = type_description<std::string, type_e::_json>;

using bool_send = signal<type_bool>;
using int_send = signal<type_int>;
using uint_send = signal<type_uint>;
using double_send = signal<type_double>;
using string_send = signal<type_string>;
using json_send = signal<type_json>;

using bool_send_ptr = std::shared_ptr<signal<type_bool>>;
using int_send_ptr = std::shared_ptr<signal<type_int>>;
using uint_send_ptr = std::shared_ptr<signal<type_uint>>;
using double_send_ptr = std::shared_ptr<signal<type_double>>;
using string_send_ptr = std::shared_ptr<signal<type_string>>;
using json_send_ptr = std::shared_ptr<signal<type_json>>;

using bool_recv = slot<type_bool>;
using int_recv = slot<type_int>;
using uint_recv = slot<type_uint>;
using double_recv = slot<type_double>;
using string_recv = slot<type_string>;
using json_recv = slot<type_json>;

using bool_recv_ptr = std::shared_ptr<slot<type_bool>>;
using int_recv_ptr = std::shared_ptr<slot<type_int>>;
using uint_recv_ptr = std::shared_ptr<slot<type_uint>>;
using double_recv_ptr = std::shared_ptr<slot<type_double>>;
using string_recv_ptr = std::shared_ptr<slot<type_string>>;
using json_recv_ptr = std::shared_ptr<slot<type_json>>;

using bool_recv_cb = slot_callback<type_bool>;
using int_recv_cb = slot_callback<type_int>;
using uint_recv_cb = slot_callback<type_uint>;
using double_recv_cb = slot_callback<type_double>;
using string_recv_cb = slot_callback<type_string>;
using json_recv_cb = slot_callback<type_json>;

using bool_recv_cb_ptr = std::shared_ptr<slot_callback<type_bool>>;
using int_recv_cb_ptr = std::shared_ptr<slot_callback<type_int>>;
using uint_recv_cb_ptr = std::shared_ptr<slot_callback<type_uint>>;
using double_recv_cb_ptr = std::shared_ptr<slot_callback<type_double>>;
using string_recv_cb_ptr = std::shared_ptr<slot_callback<type_string>>;
using json_recv_cb_ptr = std::shared_ptr<slot_callback<type_json>>;

using any_send = std::
    variant<std::monostate, bool_send_ptr, int_send_ptr, uint_send_ptr, double_send_ptr, string_send_ptr, json_send_ptr>;
using any_recv_cb = std::variant<std::monostate,
                                 bool_recv_cb_ptr,
                                 int_recv_cb_ptr,
                                 uint_recv_cb_ptr,
                                 double_recv_cb_ptr,
                                 string_recv_cb_ptr,
                                 json_recv_cb_ptr>;

using any_recv = std::
    variant<std::monostate, bool_recv_ptr, int_recv_ptr, uint_recv_ptr, double_recv_ptr, string_recv_ptr, json_recv_ptr>;

inline constexpr std::string_view invalid_type{
  "\nInvalid name {}, it must include one qualified type name.\n"  // should inject slot or signal as {}
  "Any of the following: \n"
  "bool\n"
  "int\n"
  "uint\n"
  "double\n"
  "string\n"
  "json\n"
};

template <typename return_t>
inline auto create_ipc_recv_cb(asio::io_context& ctx, std::string_view name) -> return_t {
  if (name.contains("bool")) {
    return bool_recv_cb::create(ctx, name);
  }
  if (name.contains("int")) {
    return int_recv_cb::create(ctx, name);
  }
  if (name.contains("uint")) {
    return uint_recv_cb::create(ctx, name);
  }
  if (name.contains("double")) {
    return double_recv_cb::create(ctx, name);
  }
  if (name.contains("string")) {
    return string_recv_cb::create(ctx, name);
  }
  if (name.contains("json")) {
    return json_recv_cb::create(ctx, name);
  }
  throw std::runtime_error{ fmt::format(invalid_type, name) };
}

template <typename return_t>
inline auto create_ipc_recv(asio::io_context& ctx, std::string_view name) -> return_t {
  if (name.contains("bool")) {
    return bool_recv::create(ctx, name);
  }
  if (name.contains("int")) {
    return int_recv::create(ctx, name);
  }
  if (name.contains("uint")) {
    return uint_recv::create(ctx, name);
  }
  if (name.contains("double")) {
    return double_recv::create(ctx, name);
  }
  if (name.contains("string")) {
    return string_recv::create(ctx, name);
  }
  if (name.contains("json")) {
    return json_recv::create(ctx, name);
  }
  throw std::runtime_error{ fmt::format(invalid_type, name) };
}

template <typename return_t>
inline auto create_ipc_send(asio::io_context& ctx, std::string_view name) -> return_t {
  if (name.contains("bool")) {
    return bool_send::create(ctx, name).value();
  }
  if (name.contains("int")) {
    return int_send::create(ctx, name).value();
  }
  if (name.contains("uint")) {
    return uint_send::create(ctx, name).value();
  }
  if (name.contains("double")) {
    return double_send::create(ctx, name).value();
  }
  if (name.contains("string")) {
    return string_send::create(ctx, name).value();
  }
  if (name.contains("json")) {
    return json_send::create(ctx, name).value();
  }
  throw std::runtime_error{ fmt::format(invalid_type, name) };
}
}  // namespace tfc::ipc::details
