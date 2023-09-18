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

#include <tfc/ipc/details/filter.hpp>
#include <tfc/ipc/details/type_description.hpp>
#include <tfc/ipc/enums.hpp>
#include <tfc/ipc/packet.hpp>
#include <tfc/progbase.hpp>
#include <tfc/utils/pragmas.hpp>
#include <tfc/utils/socket.hpp>
#include <tfc/stx/concepts.hpp>

namespace tfc::ipc::details {

namespace asio = boost::asio;

enum struct ipc_errors_e {
  message_to_small = 1,
  inconsistent_size = 2,
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

  [[nodiscard]] auto name() const noexcept -> std::string_view { return name_; }

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

  [[nodiscard]] static auto create(asio::io_context& ctx, std::string_view name, tfc::stx::invocable<value_t> auto&& callback)
      -> std::shared_ptr<slot_callback<type_desc>>
  {
    return std::shared_ptr<slot_callback<type_desc>>(
        new slot_callback<type_desc>{ ctx, name, std::forward<decltype(callback)>(callback) });
  }

  auto connect(std::string_view signal_name) -> std::error_code {
    if (auto error = slot_.connect(signal_name)) {
      return error;
    }
    register_read();
    return {};
  }
  [[nodiscard]] auto value() const noexcept -> std::optional<value_t> const& { return filters_.value(); }

  /**
   * @brief disconnect from signal
   */
  auto disconnect(std::string_view signal_name) { return slot_.disconnect(signal_name.data()); }

  /// \return <type>.<name> for example: bool.my_name
  [[nodiscard]] auto name_w_type() const -> std::string { return slot_.name_w_type(); }

private:
  slot_callback(asio::io_context& ctx, std::string_view name, tfc::stx::invocable<value_t> auto&& callback)
      : slot_{ ctx, name },
        filters_{ ctx, fmt::format("{}.{}", type_desc::type_name, name), std::forward<decltype(callback)>(callback) } {}
  void async_new_state(std::expected<value_t, std::error_code> value) {
    if (!value) {
      return;
    }
    // Don't retransmit transmitted things.
    // clang-format off
    auto const& last_value{ filters_.value() };
    PRAGMA_CLANG_WARNING_PUSH_OFF(-Wfloat-equal)
    if (!last_value.has_value() || value.value() != last_value) {
    PRAGMA_CLANG_WARNING_POP
      // clang-format on
      filters_(std::move(value.value()));
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
  filter::filters<value_t, std::function<void(value_t&)>> filters_;  // todo prefer some other type erasure mechanism
};

template <typename return_t, template <typename description_t> typename ipc_base_t>
struct make_any_ptr {
  static auto make(type_e type, auto&&... args) -> return_t {
    switch (type) {
      case type_e::_bool:
        return ipc_base_t<type_bool>::create(std::forward<decltype(args)>(args)...);
      case type_e::_int64_t:
        return ipc_base_t<type_int>::create(std::forward<decltype(args)>(args)...);
      case type_e::_uint64_t:
        return ipc_base_t<type_uint>::create(std::forward<decltype(args)>(args)...);
      case type_e::_double_t:
        return ipc_base_t<type_double>::create(std::forward<decltype(args)>(args)...);
      case type_e::_string:
        return ipc_base_t<type_string>::create(std::forward<decltype(args)>(args)...);
      case type_e::_json:
        return ipc_base_t<type_json>::create(std::forward<decltype(args)>(args)...);
      case type_e::unknown:
        return std::monostate{};
    }
    return {};
  }
};

using bool_signal_ptr = std::shared_ptr<signal<type_bool>>;
using int_signal_ptr = std::shared_ptr<signal<type_int>>;
using uint_signal_ptr = std::shared_ptr<signal<type_uint>>;
using double_signal_ptr = std::shared_ptr<signal<type_double>>;
using string_signal_ptr = std::shared_ptr<signal<type_string>>;
using json_signal_ptr = std::shared_ptr<signal<type_json>>;
using any_signal = std::variant<std::monostate,     //
                                bool_signal_ptr,    //
                                int_signal_ptr,     //
                                uint_signal_ptr,    //
                                double_signal_ptr,  //
                                string_signal_ptr,  //
                                json_signal_ptr>;
/// \brief any_signal foo = make_any_signal::make(type_e::bool, ctx, "name");
using make_any_signal = make_any_ptr<any_signal, signal>;

using bool_slot_ptr = std::shared_ptr<slot<type_bool>>;
using int_slot_ptr = std::shared_ptr<slot<type_int>>;
using uint_slot_ptr = std::shared_ptr<slot<type_uint>>;
using double_slot_ptr = std::shared_ptr<slot<type_double>>;
using string_slot_ptr = std::shared_ptr<slot<type_string>>;
using json_slot_ptr = std::shared_ptr<slot<type_json>>;
using any_slot = std::variant<std::monostate,   //
                              bool_slot_ptr,    //
                              int_slot_ptr,     //
                              uint_slot_ptr,    //
                              double_slot_ptr,  //
                              string_slot_ptr,  //
                              json_slot_ptr>;
/// \brief any_slot foo = make_any_slot::make(type_e::bool, ctx, "name");
using make_any_slot = make_any_ptr<any_slot, slot>;

using bool_slot_cb_ptr = std::shared_ptr<slot_callback<type_bool>>;
using int_slot_cb_ptr = std::shared_ptr<slot_callback<type_int>>;
using uint_slot_cb_ptr = std::shared_ptr<slot_callback<type_uint>>;
using double_slot_cb_ptr = std::shared_ptr<slot_callback<type_double>>;
using string_slot_cb_ptr = std::shared_ptr<slot_callback<type_string>>;
using json_slot_cb_ptr = std::shared_ptr<slot_callback<type_json>>;
using any_slot_cb = std::variant<std::monostate,      //
                                 bool_slot_cb_ptr,    //
                                 int_slot_cb_ptr,     //
                                 uint_slot_cb_ptr,    //
                                 double_slot_cb_ptr,  //
                                 string_slot_cb_ptr,  //
                                 json_slot_cb_ptr>;
/// \brief any_slot_cb foo = make_any_slot_cb::make(type_e::bool, ctx, "name", [](bool new_state){});
using make_any_slot_cb = make_any_ptr<any_slot_cb, slot_callback>;

}  // namespace tfc::ipc::details
