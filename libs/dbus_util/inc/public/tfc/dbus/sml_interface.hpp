#pragma once

#include <algorithm>
#include <concepts>
#include <map>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>

#include <fmt/format.h>
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
/// #include <memory>
/// #include <string>
/// #include <string_view>
///
/// #include <boost/asio.hpp>
/// #include <boost/sml.hpp>
/// #include <sdbusplus/asio/connection.hpp>
/// #include <sdbusplus/asio/object_server.hpp>
///
/// #include <tfc/dbus/sml_interface.hpp>
/// #include <tfc/dbus/string_maker.hpp>
/// #include <tfc/progbase.hpp>
///
/// struct run {
///   static constexpr std::string_view name{ "run" };
/// };
/// struct stop {
///   static constexpr std::string_view name{ "stop" };
/// };
///
/// struct control_modes {
///   auto operator()() {
///     using boost::sml::event;
///     using boost::sml::operator""_s;
///
///     auto table = boost::sml::make_transition_table(*"not_running"_s + event<run> = "running"_s,
///                                                    "running"_s + event<stop> = "not_running"_s);
///     return table;
///   }
/// };
///
/// auto main(int argc, char** argv) -> int {
///   tfc::base::init(argc, argv);
///   boost::asio::io_context ctx{};
///
///   /// Raw dbus connection, ipc_client also has a dbus connection which can be used through ipc_client.connection()
///   std::shared_ptr<sdbusplus::asio::connection> const dbus_connection{ std::make_shared<sdbusplus::asio::connection>(ctx)
///   };
///
///   std::shared_ptr<sdbusplus::asio::dbus_interface> const interface {
///     std::make_shared<sdbusplus::asio::dbus_interface>(dbus_connection,
///                                                       std::string{ tfc::dbus::sml::tags::path },
///                                                       tfc::dbus::make_dbus_name("example_state_machine"))
///   };
///
///   tfc::dbus::sml::interface sml_interface {
///     interface, "Log key"
///   };  // optional log key
///   // NOTE! interface struct requires to be passed by l-value like below, so the using code needs to store it like above
///
///   using state_machine_t = boost::sml::sm<control_modes, boost::sml::logger<tfc::dbus::sml::interface> >;
///
///   std::shared_ptr<state_machine_t> const state_machine{ std::make_shared<state_machine_t>(control_modes{}, sml_interface)
///   };
///
///   interface->initialize();
///
///   state_machine->process_event(run{});
///
///   ctx.run();
///
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

class node {
public:
  node() = default;

  auto get_dot_format(std::string_view label) const -> std::string {
    std::string entry_dot_format{ entry_.empty() ? "" : fmt::format(" \n entry / {} ", entry_) };
    std::string exit_dot_format{ exit_.empty() ? "" : fmt::format(" \n exit / {} ", exit_) };
    std::string color_dot_format{ color_.empty() ? "" : fmt::format(" , color = \"{}\"", color_) };
    std::string dot_format{ fmt::format("{} [ label = \" {} {} {} \" {} ]", label, label, entry_dot_format, exit_dot_format,
                                        color_dot_format) };

    return dot_format;
  }

  auto set_color(std::string_view color) -> void { color_ = color; }

  auto set_entry(std::string_view entry) -> void { entry_ = entry; }

  auto set_exit(std::string_view exit) -> void { exit_ = exit; }

private:
  std::string color_;
  std::string entry_;
  std::string exit_;
};

template <typename transition_t, typename source_state_t, typename destination_state_t>
bool constexpr is_likely_current_transition = [] {
  return std::same_as<std::remove_cvref_t<typename destination_state_t::type>, typename transition_t::dst_state> &&
         std::same_as<std::remove_cvref_t<typename source_state_t::type>, typename transition_t::src_state>;
}();

template <typename type_t>
auto get_filtered_name() -> std::string {
  auto filtered_name = get_name<type_t>();
  if (filtered_name.contains("(lambda") || filtered_name.contains("<lambda")) {
    filtered_name = "lambda";
  }
  return filtered_name;
}

template <typename type_t>
auto get_action_name() -> std::string {
  return get_filtered_name<typename type_t::action::type>();
}

template <typename T>
concept is_not_guard = tfc::stx::is_specialization_v<T, boost::sml::aux::zero_wrapper> &&
                       tfc::stx::is_specialization_v<typename T::type, boost::sml::front::not_>;

template <typename T>
struct not_guard {};

// need the inner type to be able to get the name
template <typename T>
struct not_guard<boost::sml::aux::zero_wrapper<boost::sml::front::not_<T>>> {
  using type = T;
};

template <typename type_t>
auto get_guard_name() -> std::string {
  if constexpr (is_not_guard<typename type_t::guard>) {
    using guard_info = not_guard<typename type_t::guard>;
    return fmt::format("[!{}]", get_name<typename guard_info::type>());
  }
  return fmt::format("[{}]", get_filtered_name<typename type_t::guard>());
}

template <class type_t, class source_state_t, class destination_state_t>
auto get_color(bool has_event, std::string_view last_event) -> std::string {
  if constexpr (is_likely_current_transition<type_t, source_state_t, destination_state_t>) {
    if (has_event && get_name<typename type_t::event>() == last_event) {
      return "gold";
    }
  } else if constexpr (std::same_as<std::remove_cvref_t<typename destination_state_t::type>, typename type_t::src_state>) {
    return "lightblue";
  }
  return "";
}

template <typename type_t>
auto get_action_label(const std::string& src_state) -> std::string {
  std::string action_name = get_action_name<type_t>();
  return action_name.empty() ? "" : fmt::format(R"({} [label = "{}\nentry / {}"])", src_state, src_state, action_name);
}

inline auto filter_sub_state_machine_id(std::string& state) -> void {
  std::string_view const sub_state_machine_id{ "boost::sml::back::sm<boost::sml::back::sm_policy<" };

  if (state.starts_with(sub_state_machine_id)) {
    // remove boost::sml::back::sm<boost::sml::back::sm_policy<
    state.erase(0, sub_state_machine_id.size());
    // remove >>
    state.erase(state.size() - 2, state.size());

    // only take the string up until the first <
    state.erase(state.find_first_of("<"), state.size());

    // replace :: with _
    std::string const pattern{ "::" };
    std::string const replacement{ "_" };
    size_t pos = 0;
    while ((pos = state.find(pattern, pos)) != std::string::npos) {
      state.replace(pos, pattern.length(), replacement);
      pos += replacement.length();
    }
    // place sub_state_machine in front
    state = fmt::format("sub_sm_{}", state);
  }
}

// modified version of https://boost-ext.github.io/sml/examples.html
// added color to current state
template <class type_t, class source_state_t, class destination_state_t>
auto dump_transition([[maybe_unused]] source_state_t const& src,
                     [[maybe_unused]] destination_state_t const& dst,
                     std::string_view last_event,
                     std::string& buffer,
                     std::map<std::string, node>& nodes) -> void {
  std::string src_state{ get_name<typename type_t::src_state>() };
  std::string dst_state{ get_name<typename type_t::dst_state>() };

  filter_sub_state_machine_id(src_state);
  filter_sub_state_machine_id(dst_state);

  if (dst_state == "terminate") {
    dst_state = "stop";
  }

  if (type_t::initial) {
    buffer.append("start -> ").append(src_state).append("\n");
  }

  const auto has_event = !boost::sml::aux::is_same<typename type_t::event, boost::sml::anonymous>::value;
  const auto has_guard = !boost::sml::aux::is_same<typename type_t::guard, boost::sml::front::always>::value;
  const auto has_action = !boost::sml::aux::is_same<typename type_t::action, boost::sml::front::none>::value;

  const auto is_entry =
      boost::sml::aux::is_same<typename type_t::event, boost::sml::back::on_entry<boost::sml::_, boost::sml::_>>::value;
  const auto is_exit =
      boost::sml::aux::is_same<typename type_t::event, boost::sml::back::on_exit<boost::sml::_, boost::sml::_>>::value;

  if (!is_entry && !is_exit) {
    std::string color{ get_color<type_t, source_state_t, destination_state_t>(has_event, last_event) };
    std::string guard{ has_guard ? get_guard_name<type_t>() : "" };
    std::string color_attr{ color.empty() ? "" : fmt::format(", color=\"{}\"", color) };

    std::string event_label{};
    if (has_event && has_action) {
      event_label = fmt::format("{} / {}", get_name<typename type_t::event>(), get_action_name<type_t>());
    } else if (has_event) {
      event_label = fmt::format("{}", get_name<typename type_t::event>());
    }

    buffer.append(fmt::format("{} -> {} [label=\"{} {}\"{}]\n", src_state, dst_state, event_label, guard, color_attr));
  }

  if (has_action) {
    if (is_entry) {
      auto entry_action = get_action_name<type_t>();
      nodes[src_state].set_entry(entry_action);
    }

    if (is_exit) {
      nodes[src_state].set_exit(get_action_name<type_t>());
    }
  }

  if constexpr (is_likely_current_transition<type_t, source_state_t, destination_state_t>) {
    nodes[dst_state].set_color("green");
  }

  buffer.append("\n");
}

template <template <class...> class type_t, class... types_t, class source_state_t, class destination_state_t>
auto dump_transitions(const type_t<types_t...>&,
                      source_state_t const& src,
                      destination_state_t const& dst,
                      std::string_view last_event,
                      std::string& buffer,
                      std::map<std::string, node>& nodes) -> void {
  (dump_transition<types_t>(src, dst, last_event, buffer, nodes), ...);
}

template <class state_machine_t, class source_state_t, class destination_state_t>
auto dump(source_state_t const& src, destination_state_t const& dst, std::string_view last_event) -> std::string {
  std::string buffer{ "digraph {\n\n" };
  std::map<std::string, node> nodes;
  dump_transitions(typename state_machine_t::transitions{}, src, dst, last_event, buffer, nodes);

  for (auto& [label, node_data] : nodes) {
    buffer.append(fmt::format("{} \n", node_data.get_dot_format(label)));
  }

  buffer.append("\n}\n");

  return buffer;
}
}  // namespace detail
}  // namespace tfc::dbus::sml
