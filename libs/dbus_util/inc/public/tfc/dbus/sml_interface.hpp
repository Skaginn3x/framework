#pragma once

#include <memory>
#include <string>
#include <string_view>

#include <boost/sml.hpp>

#include <tfc/dbus/sdbusplus_fwd.hpp>
#include <tfc/dbus/string_maker.hpp>
#include <tfc/sml_logger.hpp>
#include <tfc/stx/concepts.hpp>

namespace tfc::dbus::sml {

namespace tags {
static constexpr std::string_view sub_path{ "StateMachines" };
static constexpr std::string_view path{ const_dbus_path<sub_path> };
}  // namespace tags

namespace detail {

struct interface_impl {
  explicit interface_impl(std::shared_ptr<sdbusplus::asio::dbus_interface>);

  interface_impl(interface_impl const&) = delete;

  interface_impl(interface_impl&&) noexcept = default;

  auto operator=(interface_impl const&) -> interface_impl& = delete;

  auto operator=(interface_impl&&) noexcept -> interface_impl& = default;

  void on_state_change(std::string_view source_state, std::string_view destination_state, std::string_view event);

  void dot_format(std::string_view state_machine);

  std::string source_state_{};
  std::string destination_state_{};
  std::string event_{};
  std::string state_machine_dot_formatted_{};
  std::shared_ptr<sdbusplus::asio::dbus_interface> dbus_interface_{};
};

template <class state_machine_t, class source_state_t, class destination_state_t>
auto dump(source_state_t const& src, destination_state_t const& dst, std::string_view last_event) -> std::string;

template <typename type_t>
concept name_exists = requires {
  { type_t::name };
  requires std::same_as<std::string_view, std::remove_cvref_t<decltype(type_t::name)>>;
};

template <typename type_t>
constexpr auto get_name() -> std::string {
  if constexpr (name_exists<type_t>) {
    return std::string{ type_t::name };
  } else {
    return std::string{ boost::sml::aux::string<type_t>{}.c_str() };
  }
}

template <template <typename, typename> typename event_t, typename first_t, typename second_t>
auto constexpr extract_event_type(event_t<first_t, second_t> const&) noexcept -> std::string {
  return get_name<second_t>();
}

template <typename event_t>
auto constexpr extract_event_type(event_t const& event) noexcept -> std::string {
  if constexpr (tfc::stx::is_specialization_v<event_t, boost::sml::back::on_entry> ||
                tfc::stx::is_specialization_v<event_t, boost::sml::back::on_exit>) {
    return detail::extract_event_type(event);
  } else {
    return get_name<event_t>();
  }
}

}  // namespace detail

/// \brief Interface for state machine logging and dbus API
/// \example example_sml_interface.cpp
/// \code{.cpp}
/// #include <boost/asio.hpp>
/// #include <boost/sml.hpp>
/// #include <memory>
/// #include <sdbusplus/asio/connection.hpp>
/// #include <sdbusplus/asio/object_server.hpp>
/// #include <tfc/dbus/sml_interface.hpp>
/// struct state_machine {
///   auto operator()() {
///     using boost::sml::operator""_s;
///     using boost::sml::operator""_e;
///     return boost::sml::make_transition_table(*"init"_s + "set_stopped"_e = "stopped"_s);
///   }
/// };
/// int main() {
///   boost::asio::io_context ctx{};
///   auto bus = std::make_shared<sdbusplus::asio::connection>(ctx);
///   auto interface =
///       std::make_shared<sdbusplus::asio::dbus_interface>(bus, std::string{ tfc::dbus::sml::tags::path },
///       "StateMachineName");
///   tfc::dbus::sml::interface sml_interface{ interface, "Log key" };  // optional log key
///   // NOTE! interface struct requires to be passed by l-value like below, so the using code needs to store it like above
///   boost::sml::sm<state_machine, boost::sml::logger<tfc::dbus::sml::interface>> my_sm{ sml_interface };
///   return EXIT_SUCCESS;
/// }
/// \endcode
/// Get from cli example:
/// busctl --system get-property com.skaginn3x.tfc.operations.def /com/skaginn3x/StateMachines com.skaginn3x.Operations
/// StateMachine
struct interface : tfc::logger::sml_logger {
  using logger = tfc::logger::sml_logger;

  explicit interface(std::shared_ptr<sdbusplus::asio::dbus_interface> interface) : impl_{ std::move(interface) } {}

  explicit interface(std::shared_ptr<sdbusplus::asio::dbus_interface> interface, std::string_view log_key)
      : logger{ log_key }, impl_{ std::move(interface) } {}

  interface(interface const&) = delete;

  interface(interface&&) noexcept = default;

  auto operator=(interface const&) -> interface& = delete;

  auto operator=(interface&&) noexcept -> interface& = default;

  template <class state_machine_t, class event_t>
  void log_process_event([[maybe_unused]] event_t const& event) {
    last_event_ = detail::extract_event_type(event);
    logger::log_process_event<state_machine_t>(event);
  }

  template <class state_machine_t, class guard_t, class event_t>
  void log_guard(guard_t const& guard, event_t const& event, bool result) {
    logger::log_guard<state_machine_t>(guard, event, result);
  }

  template <class state_machine_t, class action_t, class event_t>
  void log_action(action_t const& action, event_t const& event) {
    logger::log_action<state_machine_t>(action, event);
  }

  template <class state_machine_t, class source_state_t, class destination_state_t>
  void log_state_change(source_state_t const& src, destination_state_t const& dst) {
    impl_.on_state_change(detail::get_name<typename source_state_t::type>(),
                          detail::get_name<typename destination_state_t::type>(), last_event_);
    impl_.dot_format(detail::dump<boost::sml::sm<state_machine_t>>(src, dst, last_event_));
    logger::log_state_change<state_machine_t>(src, dst);
  }

  std::string last_event_{};
  detail::interface_impl impl_;
};

namespace detail {

template <typename transition_t, typename source_state_t, typename destination_state_t>
bool constexpr is_likely_current_transition = [] {
  return std::same_as<std::remove_cvref_t<typename destination_state_t::type>, typename transition_t::dst_state> &&
         std::same_as<std::remove_cvref_t<typename source_state_t::type>, typename transition_t::src_state>;
}();

// modified version of https://boost-ext.github.io/sml/examples.html
// added color to current state
template <class type_t, class source_state_t, class destination_state_t>
void dump_transition([[maybe_unused]] source_state_t const& src,
                     [[maybe_unused]] destination_state_t const& dst,
                     std::string_view last_event,
                     std::string& buffer) {
  std::string src_state{ get_name<typename type_t::src_state>() };
  std::string dst_state{ get_name<typename type_t::dst_state>() };

  if (dst_state == "terminate") {
    dst_state = "stop";
  }

  if (type_t::initial) {
    buffer.append("start --> ").append(src_state).append("\n");
  }

  const auto has_event = !boost::sml::aux::is_same<typename type_t::event, boost::sml::anonymous>::value;
  const auto has_guard = !boost::sml::aux::is_same<typename type_t::guard, boost::sml::front::always>::value;
  const auto has_action = !boost::sml::aux::is_same<typename type_t::action, boost::sml::front::none>::value;

  const auto is_entry =
      boost::sml::aux::is_same<typename type_t::event, boost::sml::back::on_entry<boost::sml::_, boost::sml::_>>::value;
  const auto is_exit =
      boost::sml::aux::is_same<typename type_t::event, boost::sml::back::on_exit<boost::sml::_, boost::sml::_>>::value;

  if (!is_entry & !is_exit) {
    std::string color{};
    if constexpr (is_likely_current_transition<type_t, source_state_t, destination_state_t>) {
      if (has_event) {
        auto event = get_name<typename type_t::event>();
        if (event == last_event) {
          color = "gold";
        }
      }
    } else if constexpr (std::same_as<std::remove_cvref_t<typename destination_state_t::type>, typename type_t::src_state>) {
      color = "lightblue";
    }

    buffer.append(src_state).append(" -> ").append(dst_state).append(" ");

    std::string guard{};

    // test for guard also
    if (has_guard) {
      auto guard_n = get_name<typename type_t::guard>();
      // todo this could be done during compilation if boost::sml::aux::get_type_name would be constexpr
      if (guard_n.find("(lambda") != std::string::npos || guard_n.find("<lambda") != std::string::npos) {
        guard_n = "lambda";
      }
      guard.append("[").append(guard_n).append("]");
    }

    if (has_event) {
      if (color.empty()) {
        buffer.append(fmt::format("[label=\"{} {}\"]", get_name<typename type_t::event>(), guard));
      } else {
        buffer.append(fmt::format("[label=\"{} {}\", color = \"{}\" ]", get_name<typename type_t::event>(), guard, color));
      }
    }
  }

  if (has_action) {
    auto action_name = get_name<typename type_t::action::type>();
    if (action_name.find("(lambda") != std::string::npos || action_name.find("<lambda") != std::string::npos) {
      action_name = "lambda";
    }
    buffer.append(src_state).append("[label = \"").append(src_state).append("\\nentry / ").append(action_name).append("\"]");
  }

  if constexpr (is_likely_current_transition<type_t, source_state_t, destination_state_t>) {
    buffer.append(fmt::format("\n {} [color=\"green\"]", dst_state));
  }

  buffer.append("\n");
}

template <template <class...> class type_t, class... types_t, class source_state_t, class destination_state_t>
void dump_transitions(const type_t<types_t...>&,
                      source_state_t const& src,
                      destination_state_t const& dst,
                      std::string_view last_event,
                      std::string& buffer) {
  (dump_transition<types_t>(src, dst, last_event, buffer), ...);
}

template <class state_machine_t, class source_state_t, class destination_state_t>
auto dump(source_state_t const& src, destination_state_t const& dst, std::string_view last_event) -> std::string {
  std::string buffer{ "digraph {\n\n" };
  dump_transitions(typename state_machine_t::transitions{}, src, dst, last_event, buffer);
  buffer.append("\n}\n");
  return buffer;
}

}  // namespace detail

}  // namespace tfc::dbus::sml