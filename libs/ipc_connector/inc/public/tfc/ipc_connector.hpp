#pragma once

#include <tfc/confman.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc_connector/connect_storage.hpp>

namespace tfc::ipc {

namespace asio = boost::asio;

template <typename type_desc>
class slot_configurable {
public:
  using value_t = slot_callback<type_desc>::value_t;

  slot_configurable(asio::io_context& ctx, std::string_view name, auto&& callback)
      : slot_(slot_callback<type_desc>::create(ctx, name)), config_(ctx, name, [this, callback](auto const& config) {
          if (!config->signal_name.value().empty()) {
            slot_->init(config->signal_name.value(), callback);
          }
          config->signal_name.observe([this, callback](auto const& new_signal_name, auto const& old_signal_name) {
            if (new_signal_name.empty()) {
              slot_->disconnect(old_signal_name);
            } else {
              slot_->init(new_signal_name, callback);
            }
          });
        }) {}
  [[nodiscard]] auto value() const noexcept { return slot_->get(); }  // todo: value() or get()

  [[nodiscard]] auto config() const noexcept -> confman::config<connect_storage> const& { return config_; }

private:
  std::shared_ptr<slot_callback<type_desc>> slot_;
  confman::config<connect_storage> config_;
};

}  // namespace tfc::ipc
