#pragma once
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <tfc/confman_fwd.hpp>

namespace tfc::ipc::filter {

namespace asio = boost::asio;

enum struct filter_e : std::uint8_t {
  unknown = 0,
  invert,
  timer,
  offset,
  multiply,
  filter_out,
  // https://esphome.io/components/sensor/index.html#sensor-filters
  // todo: make the below filters
  calibrate_linear,  // https://github.com/esphome/esphome/blob/v1.20.4/esphome/components/sensor/__init__.py#L594
  median,
  quantile,
  sliding_window_moving_average,
  exponential_moving_average,
  throttle,
  throttle_average,
  delta,
  lambda,
  tfc_item,  // according to item json schema see ipc/item.hpp, TODO: implement
};

template <filter_e type, typename value_t, typename...>
struct filter;

namespace detail {
template <typename value_t>
struct any_filter_decl;
template <>
struct any_filter_decl<bool> {
  using value_t = bool;
  using type = std::variant<filter<filter_e::invert, value_t>, filter<filter_e::timer, value_t, std::chrono::steady_clock>>;
};
template <>
struct any_filter_decl<std::int64_t> {
  using value_t = std::int64_t;
  using type = std::
      variant<filter<filter_e::filter_out, value_t>, filter<filter_e::offset, value_t>, filter<filter_e::multiply, value_t>>;
};
template <>
struct any_filter_decl<std::uint64_t> {
  using value_t = std::uint64_t;
  using type = std::
      variant<filter<filter_e::filter_out, value_t>, filter<filter_e::offset, value_t>, filter<filter_e::multiply, value_t>>;
};
template <>
struct any_filter_decl<std::double_t> {
  using value_t = std::double_t;
  using type = std::
      variant<filter<filter_e::filter_out, value_t>, filter<filter_e::offset, value_t>, filter<filter_e::multiply, value_t>>;
};
template <>
struct any_filter_decl<std::string> {
  using value_t = std::string;
  using type = std::variant<filter<filter_e::filter_out, value_t>>;
};
// json?
template <typename value_t>
using any_filter_decl_t = any_filter_decl<value_t>::type;
}  // namespace detail

template <typename value_t, typename callback_t>
class filters {
public:
  filters(asio::io_context& ctx, std::string_view name, callback_t&& callback);

  /// \brief changes internal last_value state when filters have been processed
  void operator()(value_t&& value);
  [[nodiscard]] auto value() const noexcept -> std::optional<value_t> const&;

private:
  asio::io_context& ctx_;
  using config_t = std::vector<detail::any_filter_decl_t<value_t>>;
  using confman_t = confman::config<config_t, confman::file_storage<config_t>, confman::detail::config_dbus_client>;
  std::unique_ptr<confman_t, std::function<void(confman_t*)>> filters_;
  callback_t callback_;
  std::optional<value_t> last_value_{};
};

template <typename value_t>
using callback_t = std::function<void(value_t&)>;

extern template class filters<bool, callback_t<bool>>;
extern template class filters<std::uint64_t, callback_t<std::uint64_t>>;
extern template class filters<std::int64_t, callback_t<std::int64_t>>;
extern template class filters<double, callback_t<double>>;
extern template class filters<std::string, callback_t<std::string>>;

}  // namespace tfc::ipc::filter
