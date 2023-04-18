#pragma once

#include <expected>
#include <type_traits>

#include <tfc/confman/detail/common.hpp>
#include <tfc/progbase.hpp>
#include <tfc/rpc.hpp>
#include <tfc/stx/concepts.hpp>

#include <fmt/format.h>
#include <azmq/socket.hpp>
#include <boost/asio.hpp>

namespace asio = boost::asio;

namespace tfc::confman::detail {

using client_method_alive = glz::rpc::client_method_t<method::alive::tag.data_, method::alive, method::alive_result>;
using client_method_get_ipcs =
    glz::rpc::client_method_t<method::get_ipcs::tag.data_, method::get_ipcs, method::get_ipcs_result>;
using client_t = tfc::rpc::client<glz::rpc::client<client_method_alive, client_method_get_ipcs>>;

class config_rpc_client {
public:
  explicit config_rpc_client(asio::io_context& ctx, std::string_view key)
      : topic_{ fmt::format("{}.{}.{}", tfc::base::get_exe_name(), tfc::base::get_proc_name(), key) },
        client_{ ctx, rpc_socket_path }, notifications_{ ctx } {
    notifications_.connect(std::string{ notify_socket_path.data(), notify_socket_path.size() });
    notifications_.set_option(azmq::socket::subscribe(topic_));
    notifications_.async_receive([this](auto const& err, auto const& msg, auto bytes) { on_notification(err, msg, bytes); });
  }

  template <stx::basic_fixed_string method_name>
  void request(auto&& params, auto&& callback) {
    client_.async_request<method_name.data_>(
        std::forward<decltype(params)>(params),
        [callback](auto const& result, glz::rpc::jsonrpc_id_type const&) { std::invoke(callback, result); });
  }

  void alive(std::string_view schema,
             std::string_view defaults,
             stx::nothrow_invocable<std::expected<method::alive_result, glz::rpc::error>> auto&& callback) {
    request<"alive">(tfc::confman::detail::method::alive{ .schema = schema, .defaults = defaults, .identity = topic_ },
                     std::forward<decltype(callback)>(callback));
  }

  auto topic() const noexcept -> std::string_view { return topic_; }

  void subscribe(std::invocable<std::expected<std::string_view, std::error_code>> auto&& callback) {
    on_notify_ = std::forward<decltype(callback)>(callback);
  }

private:
  void on_notification(std::error_code const& err, azmq::message const& msg, std::size_t bytes_received) {
    if (err) {
      on_notify_(std::unexpected(err));
      return;
    }
    std::string_view const msg_str{ static_cast<char const*>(msg.data()), bytes_received };
    if (!msg_str.starts_with(topic_)) [[unlikely]] {
      on_notify_(std::unexpected(std::make_error_code(std::errc::no_message)));  // todo custom error code
      return;
    }
    on_notify_(msg_str.substr(topic_.size()));

    notifications_.async_receive([this](auto const& inner_err, auto const& inner_msg, auto inner_bytes) {
      on_notification(inner_err, inner_msg, inner_bytes);
    });
  }

  std::string topic_{};
  client_t client_;
  azmq::sub_socket notifications_;
  std::function<void(std::expected<std::string_view, std::error_code>)> on_notify_{
    [](std::expected<std::string_view, std::error_code>) {}
  };
};

}  // namespace tfc::confman::detail
