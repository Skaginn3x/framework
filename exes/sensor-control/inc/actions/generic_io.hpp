#pragma once

#include <functional>
#include <string_view>
#include <boost/ut.hpp>

#include <boost/asio/async_result.hpp>
#include <boost/asio/io_context.hpp>
#include <glaze/core/common.hpp>

#include <tfc/ipc.hpp>

#include "api.hpp"
#include "tfc/ipc/details/dbus_client_iface.hpp"

namespace tfc::sensor::control::actions {
namespace asio = boost::asio;

class generic_io {
public:
  struct config {
    using type = generic_io;
    bool stop_motor{ false };
    static constexpr std::string_view action_type{ "generic_io" };
  };

  generic_io(asio::io_context& io_context, ipc_ruler::ipc_manager_client& client, config const& cfg, api&& api)
    : ctx_{ io_context }, ipc_client_{ client },  cfg_{ cfg }, api_{ std::move(api) } {
  }

  auto activate(asio::completion_token_for<void(std::error_code)> auto&& token) -> decltype(auto) {
    // todo this deadlocks if slot is disconnected
    return asio::async_compose<decltype(token), void(std::error_code)>([this, first_call = true](auto& self, std::error_code err = {}, std::size_t = 0) mutable {
      if (err) {
        self.complete(err);
        return;
      }
      if (hacky_finished_) {
        hacky_finished_ = false;
        if (cfg_.stop_motor) {
          api_.start_motor();
        }
        active_.async_send(false, [](std::error_code, std::size_t){/* ignore for now */});
        self.complete({});
        return;
      }
      if (first_call) { // This is the actual action to be taken
        first_call = false;
        active_.async_send(true, std::move(self));
        if (cfg_.stop_motor) {
          api_.stop_motor();
        }
        return;
      }
      hacky_polling_timer_.expires_after(std::chrono::milliseconds{ 500 });
      hacky_polling_timer_.async_wait(std::move(self));
    }, std::forward<decltype(token)>(token), ctx_);
  }

private:
  auto on_complete(bool const new_value) -> void {
    if (new_value) {
      hacky_finished_ = true;
    }
  }

  asio::io_context& ctx_;
  ipc_ruler::ipc_manager_client& ipc_client_;
  config const& cfg_;
  api api_;

  tfc::ipc::bool_signal active_{ctx_, ipc_client_, "active", "Action is active and awaiting acknowledgment" };
  tfc::ipc::bool_slot complete_{ctx_, ipc_client_, "complete", "Action is complete", std::bind_front(&generic_io::on_complete, this) };
  bool hacky_finished_{ false };
  asio::steady_timer hacky_polling_timer_{ ctx_ };

};
} // namespace tfc::sensor::control::actions

namespace glz {

template <typename>
struct meta;

template <>
struct meta<tfc::sensor::control::actions::generic_io::config> {
  using type = tfc::sensor::control::actions::generic_io::config;
  static constexpr auto name{ "generic_io" };
  static constexpr auto value{ object("stop_motor", &type::stop_motor,
                                           "Stop motor while awaiting complete indicator",
                                           "action_type", &type::action_type, "This action type") };
};

}
