#pragma once

#include <expected>
#include <filesystem>
#include <ranges>
#include <string_view>
#include <unordered_map>

#include <fmt/format.h>
#include <azmq/socket.hpp>
#include <boost/asio.hpp>
#include <glaze/glaze.hpp>

#include <tfc/confman/detail/common.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc/glaze_meta.hpp>
#include <tfc/logger.hpp>
#include <tfc/progbase.hpp>
#include <tfc/rpc.hpp>
#include <tfc/stx/basic_fixed_string.hpp>
#include <tfc/stx/concepts.hpp>

namespace asio = boost::asio;

namespace tfc::confman::detail {

static std::string_view constexpr default_config_filename{ "/var/tfc/config/confman.json" };

using server_method_alive = glz::rpc::server_method_t<method::alive::tag.data_, method::alive, method::alive_result>;
using server_method_get_ipcs =
    glz::rpc::server_method_t<method::get_ipcs::tag.data_, method::get_ipcs, method::get_ipcs_result>;
using server_t = tfc::rpc::server<glz::rpc::server<server_method_alive, server_method_get_ipcs>>;

class config_rpc_server {
public:
  using application_id_t = std::string;
  using config_t = glz::raw_json;
  using config_schema_t = glz::raw_json;
  struct map_obj_t {
    config_t config{};
    config_schema_t schema{};
    struct glaze {
      // clang-format off
      static auto constexpr value{ glz::object(
          "config", &map_obj_t::config, "Config object"
          "schema", &map_obj_t::schema, "Config object metadata"
          ) };
      // clang-format on
      static std::string_view constexpr name{ "Config object metadata container" };
    };
  };

  explicit config_rpc_server(asio::io_context& ctx, std::string_view filename = default_config_filename)
      : server_{ ctx, rpc_socket_path }, notifications_{ ctx }, config_file_{ filename }, logger_("config_rpc_server") {
    server_.converter().on<method::alive::tag.data_>(std::bind_front(&config_rpc_server::on_alive_request, this));
    server_.converter().on<method::get_ipcs::tag.data_>(std::bind_front(&config_rpc_server::get_ipcs_request, this));
    notifications_.bind(std::string{ notify_socket_path.data(), notify_socket_path.size() });

    std::filesystem::create_directories(config_file_.parent_path());
    glz::read_file_json(config_, config_file_.string());
  }

  auto update(std::string_view key, glz::raw_json_view json) -> std::error_code {
    if (!config_.contains(key.data())) {
      logger_.warn("Config does not contain key {}, rejecting request", key);
      return std::make_error_code(std::errc::argument_out_of_domain);
    }
    config_.at(key.data()).config = json.str;
    auto shared_buffer = std::make_shared<std::string>(std::string{ key }.append(json.str));
    notifications_.async_send(asio::buffer(*shared_buffer), [this, shared_buffer](std::error_code err, std::size_t) {
      if (err) {
        logger_.error("RPC server notification send error: %s\n", err.message().data());
      }
    });
    write_to_file();
    return {};
  }

private:
  auto on_alive_request(method::alive const& req) -> std::expected<method::alive_result, glz::rpc::error> {
    logger_.trace("Config registering: {}", req.identity);
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
  auto get_ipcs_request(method::get_ipcs const& req) -> std::expected<method::get_ipcs_result, glz::rpc::error> {
    // todo write_json looks wrong, how go use the enumerate tuple directly?
    std::string const type_contains = req.type == ipc::type_e::unknown ? "" : glz::write_json(req.type);

    method::get_ipcs_result result{};
    if (req.direction == ipc::direction_e::unknown) {
      auto ipcs{ config_ | std::views::filter([&req, &type_contains](auto const& map_item) {
                   return (map_item.first.contains(ipc::signal_tag) || map_item.first.contains(ipc::slot_tag)) &&
                          map_item.first.contains(type_contains) && map_item.first.contains(req.name_contains);
                 }) };

      for (auto const& ipc_item : ipcs) {
        auto const ipc_item_config{ glz::read_json<glz::json_t>(ipc_item.second.config.str) };
        if (ipc_item_config.has_value() && ipc_item_config->contains("name")) {
          result.push_back(ipc_item_config->operator[]("name").as<std::string>());
        }
      }
    }

    return result;
  }

  auto write_to_file() -> std::error_code {
    std::string buffer{};
    glz::write<glz::opts{ .prettify = true }>(config_, buffer);
    auto glz_err{ glz::buffer_to_file(buffer, config_file_.string()) };
    if (glz_err != glz::error_code::none) {
      logger_.error(R"(Error: "{}" writing to file: "{}")", glz::write_json(glz_err), config_file_.string());
      return std::make_error_code(std::errc::io_error);
      // todo implicitly convert glaze error_code to std::error_code
    }
    return {};
  }

  server_t server_;
  azmq::pub_socket notifications_;

  std::filesystem::path config_file_{};
  std::unordered_map<application_id_t, map_obj_t> config_{};
  tfc::logger::logger logger_;
};

}  // namespace tfc::confman::detail
