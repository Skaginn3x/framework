#pragma once

#include <expected>
#include <filesystem>
#include <unordered_map>

#include <tfc/confman/detail/common.hpp>
#include <tfc/progbase.hpp>
#include <tfc/rpc.hpp>
#include <tfc/stx/concepts.hpp>

#include <fmt/format.h>
#include <fmt/printf.h>
#include <azmq/socket.hpp>
#include <boost/asio.hpp>
#include <glaze/glaze.hpp>

namespace asio = boost::asio;

namespace tfc::confman::detail {

static std::string_view constexpr default_config_filename{ "/var/tfc/config/confman.json" };

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

  explicit config_rpc_server(asio::io_context& ctx, std::string_view filename = default_config_filename)
      : server_{ ctx, rpc_socket_path }, notifications_{ ctx }, config_file_{ filename } {
    server_.converter().on<method::alive_tag.data_>(
        [this](auto&& PH1) { return on_alive_request(std::forward<decltype(PH1)>(PH1)); });
    notifications_.bind(std::string{ notify_socket_path.data(), notify_socket_path.size() });

    std::filesystem::create_directories(config_file_.parent_path());
    glz::read_file_json(config_, config_file_.string());
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
    write_to_file();
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
      write_to_file();
    }
    return method::alive_result{ .config = config_.at(req.identity).config.str };
  }

  auto write_to_file() -> std::error_code {
    std::string buffer{};
    glz::write<glz::opts{ .prettify = true }>(config_, buffer);
    auto glz_err{ glz::buffer_to_file(buffer, config_file_.string()) };
    if (glz_err != glz::error_code::none) {
      return std::make_error_code(std::errc::io_error);
      // todo implicitly convert glaze error_code to std::error_code
      // todo at least add logging
    }
    return {};
  }

  server_t server_;
  azmq::pub_socket notifications_;

  std::filesystem::path config_file_{};
  std::unordered_map<application_id_t, map_obj_t> config_{};
};

}  // namespace tfc::confman::detail
