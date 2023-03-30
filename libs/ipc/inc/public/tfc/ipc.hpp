#pragma once

#include <array>
#include <concepts>
#include <expected>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <string_view>

#include <fmt/format.h>
#include <azmq/socket.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/system/error_code.hpp>

#include <tfc/ipc/packet.hpp>
#include <tfc/logger.hpp>
#include <tfc/progbase.hpp>
#include <tfc/utils/pragmas.hpp>

namespace tfc::ipc {

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
};

/**@brief
 * Base class for signal and slot. Contains naming
 * and shared ptr factory constructors.
 * */
class transmission_base {
public:
  explicit transmission_base(std::string_view name) : name_(name) {}

protected:
  [[nodiscard]] auto endpoint() const -> std::string { return fmt::format("ipc://{}", ipc_path(name_)); }
  [[nodiscard]] static auto endpoint_connect(const std::string_view& name) -> std::string {
    return fmt::format("ipc://{}", ipc_path(name));
  }
  static constexpr std::string_view path_prefix{ "/tmp/" };

private:
  std::string name_;  // name of signal/slot
  [[nodiscard]] static auto ipc_path(const std::string_view& name) -> std::string {
    return fmt::format("{}{}.{}.{}", path_prefix, base::get_exe_name(), base::get_proc_name(), name);
  }
};

/**@brief
 * Signal publishing class.
 * */
template <typename type_desc>
class signal : public transmission_base, public std::enable_shared_from_this<signal<type_desc>> {
public:
  using value_t = typename type_desc::value_t;
  using packet_t = packet<value_t, type_desc::value_e>;

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
    packet_t packet{ .value = last_value_ };
    const auto attempted_serialize{ packet_t::serialize(packet) };
    if (!attempted_serialize.has_value()) {
      return attempted_serialize.error();
    }
    const auto serialized = attempted_serialize.value();
    std::size_t size = socket_.send(asio::buffer(serialized, serialized.size()));
    if (size != serialized.size()) {
      // todo: create custom error codes and return here
      std::terminate();
    }
    return {};
  }

  /// @brief send value to subscriber
  /// @param value is sent
  auto async_send(value_t const& value, std::invocable<std::error_code, std::size_t> auto&& callback) -> void {
    last_value_ = value;
    packet_t packet{ .value = last_value_ };
    const auto attempted_serialize{ packet_t::serialize(packet) };
    if (!attempted_serialize.has_value()) {
      callback(attempted_serialize.error(), 0);
      return;
    }
    auto serialized = std::make_shared<std::vector<std::byte>>(attempted_serialize.value());
    socket_.async_send(asio::buffer(serialized->data(), serialized->size()),
                       [callback, buffer = serialized](std::error_code error, size_t size) { callback(error, size); });
  }

private:
  signal(asio::io_context& ctx, std::string_view name)
      : transmission_base(name), last_value_(), timer(ctx), socket_(ctx),
        socket_monitor_(socket_.monitor(ctx, ZMQ_EVENT_HANDSHAKE_SUCCEEDED)){};

  auto init() -> std::error_code {
    boost::system::error_code error_code;
    socket_.bind(endpoint(), error_code);
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
    timer.expires_after(std::chrono::milliseconds(1));
    timer.async_wait([this](std::error_code const& error) {
      if (error) {
        return;
      }
      async_send(last_value_, [&](std::error_code error, size_t) {
        if (error) {
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
  boost::asio::steady_timer timer;
  azmq::pub_socket socket_;
  azmq::socket socket_monitor_;
};

/**@brief slot
 *
 * */
template <typename type_desc>
class slot : public transmission_base {
public:
  using value_t = typename type_desc::value_t;
  using packet_t = packet<value_t, type_desc::value_e>;
  slot(asio::io_context& ctx, std::string_view name) : transmission_base(name), socket_(ctx) {}
  /**
   * @brief
   * connect to the signal indicated by name
   * */
  auto connect(std::string_view signal_name) -> std::error_code {
    // TODO: Find out if these mutexes inside optimize single threaded are really needed
    socket_ = azmq::sub_socket(socket_.get_io_context(), true);
    boost::system::error_code error_code;
    std::string const socket_path = fmt::format("ipc://{}{}", path_prefix, signal_name);
    socket_.connect(socket_path, error_code);
    if (error_code) {
      return error_code;
    }
    socket_.set_option(azmq::socket::subscribe(""), error_code);
    if (error_code) {
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

  /**
   * @brief schedule an async_read of the slot
   * */
  auto async_receive(auto callback) -> void {
    socket_.async_receive(
        [callback](std::error_code const& err_code, azmq::message& msg, size_t bytes_received) {
          if (err_code) {
            callback(std::unexpected(err_code));
            return;
          }
          if (bytes_received < packet_t::header_size()) {
            // TODO: return some sane code here
            // callback(std::unexpected({}));
            return;
          }
          auto packet = packet_t::deserialize(std::span(static_cast<std::byte const*>(msg.data()), bytes_received));
          callback(packet.value);
        },
        0);
  }

private:
  azmq::sub_socket socket_;
};

template <typename type_desc>
class slot_callback : public std::enable_shared_from_this<slot_callback<type_desc>> {
public:
  using value_t = type_desc::value_t;

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
  [[nodiscard]] auto get() const noexcept -> value_t const& { return last_value_; }

private:
  slot_callback(asio::io_context& ctx, std::string_view name) : slot_(ctx, name) {}
  void async_new_state(std::expected<value_t, std::error_code> value) {
    if (!value) {
      return;
    }
    // Don't retransmit transmitted things.
    // clang-format off
    PRAGMA_CLANG_WARNING_PUSH_OFF(-Wfloat-equal)
    if (value.value() != last_value_) {
      PRAGMA_CLANG_WARNING_POP
      // clang-format on
      last_value_ = value.value();
      cb_(last_value_);
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
  value_t last_value_{};
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

}  // namespace tfc::ipc
