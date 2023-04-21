#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <boost/asio.hpp>
#include <glaze/glaze.hpp>
#include <gpiod.hpp>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc_connector.hpp>
#include <tfc/logger.hpp>

namespace asio = boost::asio;

struct pin {
  template <tfc::confman::observable_type conf_param_t>
  using observable = tfc::confman::observable<conf_param_t>;

  observable<gpiod::line::direction> direction{ gpiod::line::direction::AS_IS };
  struct in {
    observable<gpiod::line::edge> edge{ gpiod::line::edge::NONE };
    observable<gpiod::line::bias> bias{ gpiod::line::bias::AS_IS };
  };
  struct out {
    enum struct force_e : std::uint8_t {
      as_is = 0,
      on = 1,
      off = 2,
      save_on = 3,
      save_off = 4,
    };
    observable<force_e> force{ force_e::as_is };
    // todo not sure that raspberry pi has fet buffer, is this used
    observable<gpiod::line::drive> drive{ gpiod::line::drive::OPEN_SOURCE };
  };
  std::variant<std::monostate, in, out> in_or_out{ std::monostate{} };
};

template <>
struct glz::meta<pin> {
  //  clang-format off
  static constexpr auto value{
    glz::object("direction", &pin::direction, "Input or Output", "in_or_out", &pin::in_or_out, "Instance")
  };
  // clang-format on
  static constexpr std::string_view name{ "Pin" };
};
template <>
struct glz::meta<pin::in> {
  using in = pin::in;
  //  clang-format off
  static constexpr auto value{ glz::object("edge", &in::edge, "Event edge detection", "bias", &in::bias, "Voltage bias") };
  // clang-format on
  static constexpr std::string_view name{ "Pin Input" };
};
template <>
struct glz::meta<pin::out> {
  using out = pin::out;
  //  clang-format off
  static constexpr auto value{
    glz::object("force", &out::force, "Force output to state", "drive", &out::drive, "Transistor driver")
  };
  // clang-format on
  static constexpr std::string_view name{ "Pin Output" };
};
template <>
struct glz::meta<pin::out::force_e> {
  using enum pin::out::force_e;
  //  clang-format off
  static constexpr auto value{ glz::enumerate("as_is",
                                              as_is,  // "Do not make any changes",
                                              "on",
                                              on,  // "Force to ON then go back to as_is",
                                              "off",
                                              off,  // "Force to OFF then go back to as_is",
                                              "save_on",
                                              save_on,  // "Force indefinitely to ON",
                                              "save_off",
                                              save_off  //, "Force indefinitely to OFF"
                                              ) };
  // clang-format on
  static constexpr std::string_view name{ "Pin output force state" };
};
template <>
struct glz::meta<gpiod::line::direction> {
  using enum gpiod::line::direction;
  //  clang-format off
  static constexpr auto value{ glz::enumerate("as_is",
                                              AS_IS,  //"Request the pin, but don't change current direction.",
                                              "input",
                                              INPUT,  //"Direction is input - we're reading the state of a GPIO pin.",
                                              "output",
                                              OUTPUT  //, "Direction is output - we're driving the GPIO line."
                                              ) };
  // clang-format on
  static constexpr std::string_view name{ "Pin direction" };
};
template <>
struct glz::meta<gpiod::line::edge> {
  using enum gpiod::line::edge;
  //  clang-format off
  static constexpr auto value{ glz::enumerate("none",
                                              NONE,  // "Pin edge detection is disabled.",
                                              "rising",
                                              RISING,  // "Pin detects rising edge events.",
                                              "falling",
                                              FALLING,  // "Line detect falling edge events.",
                                              "both",
                                              BOTH  //, "Line detects both rising and falling edge events."
                                              ) };
  // clang-format on
  static constexpr std::string_view name{ "Pin edge detection" };
};
template <>
struct glz::meta<gpiod::line::bias> {
  using enum gpiod::line::bias;
  //  clang-format off
  static constexpr auto value{ glz::enumerate("as_is",
                                              AS_IS,  // "Don't change the bias setting when applying line config.",
                                              "unknown",
                                              UNKNOWN,  // "The internal bias state is unknown.",
                                              "disabled",
                                              DISABLED,  // "The internal bias is disabled.",
                                              "pull_up",
                                              PULL_UP,  // "The internal pull-up bias is enabled.",
                                              "pull_down",
                                              PULL_DOWN  //, "The internal pull-down bias is enabled."
                                              ) };
  // clang-format on
  static constexpr std::string_view name{ "Pin voltage bias" };
};
template <>
struct glz::meta<gpiod::line::drive> {
  using enum gpiod::line::drive;
  //  clang-format off
  static constexpr auto value{ glz::enumerate("push_pull",
                                              PUSH_PULL,  // "Drive setting is push-pull.",
                                              "open_drain",
                                              OPEN_DRAIN,  // "Pin output is open-drain.",
                                              "open_source",
                                              OPEN_SOURCE  //, "Pin output is open-source."
                                              ) };
  // clang-format on
  static constexpr std::string_view name{ "Pin transistor driver setup" };
};

namespace tfc {

class gpio {
public:
  using ipc_output_t = tfc::ipc::bool_send_ptr;
  using ipc_input_t = tfc::ipc::bool_recv_conf_cb;
  using send_or_recv_t = std::variant<std::monostate, std::shared_ptr<ipc_input_t>, ipc_output_t>;  // todo why shared
  using config_t = tfc::confman::config<std::vector<pin>>;
  using pin_index_t = std::size_t;

  gpio(asio::io_context& ctx, std::filesystem::path const& char_device);

  void init(config_t const&);

private:
  void pin_direction_change(pin_index_t, gpiod::line::direction new_value, gpiod::line::direction old_value) noexcept;
  void pin_edge_change(pin_index_t, gpiod::line::edge new_value, gpiod::line::edge old_value) noexcept;
  void pin_bias_change(pin_index_t, gpiod::line::bias new_value, gpiod::line::bias old_value) noexcept;
  void pin_force_change(pin_index_t, pin::out::force_e new_value, pin::out::force_e old_value) noexcept;
  void pin_drive_change(pin_index_t, gpiod::line::drive new_value, gpiod::line::drive old_value) noexcept;

  void pin_event(pin_index_t, bool state) noexcept;

  void ipc_event(pin_index_t, bool state) noexcept;

  void chip_ready_to_read(std::error_code const&) noexcept;

  asio::io_context& ctx_;
  gpiod::chip chip_;
  config_t config_;
  std::vector<send_or_recv_t> pins_;
  tfc::logger::logger logger_;
  boost::asio::posix::stream_descriptor chip_asio_;
  boost::asio::mutable_buffer chip_asio_buffer_{};
};

}  // namespace tfc