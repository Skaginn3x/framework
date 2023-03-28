#pragma once

#include <expected>
#include <unordered_map>

#include <tfc/confman/detail/common.hpp>
#include <tfc/progbase.hpp>
#include <tfc/rpc.hpp>
#include <tfc/stx/concepts.hpp>

#include <fmt/format.h>
#include <azmq/socket.hpp>
#include <boost/asio.hpp>
#include <glaze/glaze.hpp>

namespace asio = boost::asio;

namespace tfc::confman::detail {

using server_t = tfc::rpc::server<
    glz::rpc::server<glz::rpc::server_method_t<method::alive_tag.data_, method::alive, method::alive_result>>>;

class config_rpc_server {
public:
  using application_id_t = std::string;
  using config_t = glz::raw_json;
  using config_schema_t = glz::raw_json;
  struct map_obj_t {
    config_t config{};
    config_schema_t schema{};
    struct glaze {
      static auto constexpr value{ glz::object("config", &map_obj_t::config, "schema", &map_obj_t::schema) };
    };
  };

  explicit config_rpc_server(asio::io_context& ctx) : server_{ ctx, rpc_socket }, notifications_{ ctx } {
    server_.converter().on<method::alive_tag.data_>(
        [this](auto&& PH1) { return on_alive_request(std::forward<decltype(PH1)>(PH1)); });
    notifications_.bind(fmt::format("ipc://{}", notify_socket));
  }

  auto update(std::string_view key, glz::raw_json_view json) -> std::error_code {
    if (!config_.contains(key.data())) {
      return std::make_error_code(std::errc::argument_out_of_domain);
    }
    config_.at(key.data()).config = json.str;
    auto shared_buffer = std::make_shared<std::string>(std::string{ key }.append(json.str));
    notifications_.async_send(asio::buffer(*shared_buffer), [shared_buffer](std::error_code err, std::size_t) {
      if (err) {
        fmt::fprintf(stderr, "RPC server notification send error: %s\n", err.message().data());
      }
    });
    return {};
  }

private:
  auto on_alive_request(method::alive const& req) -> std::expected<method::alive_result, glz::rpc::error> {
    if (!config_.contains(req.identity)) {
      auto [unused, inserted] =
          config_.emplace(req.identity, map_obj_t{ .config = req.defaults.str, .schema = req.schema.str });
      if (!inserted) {
        return std::unexpected(glz::rpc::error{ glz::rpc::error_e::internal,
                                                fmt::format("Unable to insert key: {} to config map", req.identity) });
      }
    }
    return method::alive_result{ .config = config_.at(req.identity).config.str };
  }

  server_t server_;
  azmq::pub_socket notifications_;

  std::unordered_map<application_id_t, map_obj_t> config_{};
};

}  // namespace tfc::confman::detail
