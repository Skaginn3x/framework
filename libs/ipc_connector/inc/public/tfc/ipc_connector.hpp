#pragma once

#include <tfc/confman.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc/glaze_meta.hpp>
#include <tfc/ipc_connector/storage/connect.hpp>
#include <tfc/ipc_connector/storage/name.hpp>

namespace tfc::ipc {

namespace asio = boost::asio;

template <typename type_desc>
class slot_configurable {
public:
  using value_t = slot_callback<type_desc>::value_t;
  static constexpr std::string_view type_name{ type_desc::type_name };
  static auto constexpr direction_v = slot_callback<type_desc>::direction_v;

  slot_configurable(asio::io_context& ctx, std::string_view name, auto&& callback)
      : slot_(slot_callback<type_desc>::create(ctx, name)),
        name_config_{ ctx, fmt::format("{}.{}", self_name, slot_->name()), [](auto&) {},
                      storage::name<self_name, type_name>{ .value =
                                                               tfc::confman::read_only<std::string>{ slot_->name() } } },
        config_(ctx, name, [this, callback](auto const& config) {
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

  [[nodiscard]] auto config() const noexcept -> confman::config<storage::connect> const& { return config_; }

private:
  static constexpr std::string_view self_name{ slot_tag };
  std::shared_ptr<slot_callback<type_desc>> slot_;
  confman::config<storage::name<self_name, type_name>> name_config_;
  confman::config<storage::connect> config_;
};

template <typename type_desc>
class signal_exposed {
public:
  using value_t = signal<type_desc>::value_t;
  static constexpr std::string_view type_name{ type_desc::type_name };
  static auto constexpr direction_v = signal<type_desc>::direction_v;

  signal_exposed(asio::io_context& ctx, std::string_view name)
      : signal_{ signal<type_desc>::create(ctx, name).value() }, name_config_{
          ctx, fmt::format("{}.{}", self_name, signal_->name()), [](auto&) {},
          storage::name<self_name, type_name>{ .value = tfc::confman::read_only<std::string>{ signal_->name() } }
        } {}

private:
  static constexpr std::string_view self_name{ signal_tag };
  std::shared_ptr<signal<type_desc>> signal_;
  confman::config<storage::name<self_name, type_name>> name_config_;
};

using bool_recv_conf_cb = slot_configurable<type_bool>;
using int_recv_conf_cb = slot_configurable<type_int>;
using uint_recv_conf_cb = slot_configurable<type_uint>;
using double_recv_conf_cb = slot_configurable<type_double>;
using string_recv_conf_cb = slot_configurable<type_string>;
using json_recv_conf_cb = slot_configurable<type_json>;

using bool_send_exposed = signal_exposed<type_bool>;
using int_send_exposed = signal_exposed<type_int>;
using uint_send_exposed = signal_exposed<type_uint>;
using double_send_exposed = signal_exposed<type_double>;
using string_send_exposed = signal_exposed<type_string>;
using json_send_exposed = signal_exposed<type_json>;

}  // namespace tfc::ipc
