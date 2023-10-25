#pragma once

#include <memory>
#include <sstream>
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

template <class state_machine_t, class state_t>
void dump(state_t const& current_state, std::ostream& out) noexcept;

template <template <typename, typename> typename event_t, typename first_t, typename second_t>
auto constexpr extract_event_type(event_t<first_t, second_t> const&) noexcept -> std::string {
  return boost::sml::aux::get_type_name<second_t>();
}

template <typename event_t>
auto constexpr extract_event_type(event_t const& event) noexcept -> std::string {
  if constexpr (tfc::stx::is_specialization_v<event_t, boost::sml::back::on_entry> ||
                tfc::stx::is_specialization_v<event_t, boost::sml::back::on_exit>) {
    return detail::extract_event_type(event);
  } else {
    return boost::sml::aux::get_type_name<event_t>();
  }
}

}  // namespace detail

/// \brief Interface for state machine logging and dbus API
/// \example
/// \code{.cpp}
/// #include <tfc/dbus/sml_interface.hpp>
/// struct state_machine {
///   auto operator()() {
///     using boost::sml::operator""_s;
///     using boost::sml::operator""_e;
///     return boost::sml::make_transition_table(
///         * "init"_s + "set_stopped"_e = "stopped"_s
///     );
///   }
/// };
///
/// int main() {
///   boost::asio::io_context ctx{};
///   auto bus = std::make_shared<sdbusplus::asio::connection>(ctx);
///   auto interface = std::make_shared<sdbusplus::asio::dbus_interface>(bus, std::string{ tfc::dbus::sml::tags::path },
///   "StateMachineName"); tfc::dbus::sml::interface sml_interface{ interface, "Log key" }; // optional log key
///   // NOTE! interface struct requires to be passed by l-value like below, so the using code needs to store it like above
///   boost::sml::sm<state_machine, boost::sml::logger<tfc::dbus::sml::interface>> sm{ sml_interface };
///   return EXIT_SUCCESS;
/// }
/// \endcode
/// Get from cli example:
/// busctl --system get-property com.skaginn3x.tfc.operations.def /com/skaginn3x/StateMachines com.skaginn3x.Operations
/// StateMachine
struct interface : tfc::logger::sml_logger {
  using logger = tfc::logger::sml_logger;

  explicit interface(std::shared_ptr<sdbusplus::asio::dbus_interface> interface) : logger{}, impl_{ std::move(interface) } {}
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
    impl_.on_state_change(src.c_str(), dst.c_str(), last_event_);
    std::stringstream iss{};
    detail::dump<boost::sml::sm<state_machine_t>>(dst, iss);
    impl_.dot_format(iss.str());
    logger::log_state_change<state_machine_t>(src, dst);
  }

  std::string last_event_{};
  detail::interface_impl impl_;
};

namespace detail {

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

// modified version of https://boost-ext.github.io/sml/examples.html
// added color to current state
template <class type_t, class state_t>
void dump_transition([[maybe_unused]] state_t const& current_state, std::ostream& out) noexcept {
  std::string src_state{ get_name<typename type_t::src_state>() };
  std::string dst_state{ get_name<typename type_t::dst_state>() };

  if (dst_state == "X") {
    dst_state = "[*]";
  }

  if (type_t::initial) {
    out << "[*] --> " << src_state << "\n";
  }

  const auto has_event = !boost::sml::aux::is_same<typename type_t::event, boost::sml::anonymous>::value;
  const auto has_guard = !boost::sml::aux::is_same<typename type_t::guard, boost::sml::front::always>::value;
  const auto has_action = !boost::sml::aux::is_same<typename type_t::action, boost::sml::front::none>::value;

  const auto is_entry =
      boost::sml::aux::is_same<typename type_t::event, boost::sml::back::on_entry<boost::sml::_, boost::sml::_>>::value;
  const auto is_exit =
      boost::sml::aux::is_same<typename type_t::event, boost::sml::back::on_exit<boost::sml::_, boost::sml::_>>::value;

  if (is_entry || is_exit) {
    out << src_state;
  } else {  // state to state transition
    out << src_state << " --> " << dst_state;
  }

  if constexpr (std::same_as<std::remove_cvref_t<typename state_t::type>, typename type_t::dst_state>) {
    out << " #limegreen";
  }

  if (has_event || has_guard || has_action) {
    out << " :";
  }

  if (has_event) {
    auto event = get_name<typename type_t::event>();
    if (is_entry) {
      event = "entry";
    } else if (is_exit) {
      event = "exit";
    }
    out << " " << event;
  }

  if (has_guard) {
    out << " [" << get_name<typename type_t::guard::type>() << "]";
  }

  if (has_action) {
    out << " / " << get_name<typename type_t::action::type>();
  }

  out << "\n";
}

template <template <class...> class type_t, class... types_t, class state_t>
void dump_transitions(const type_t<types_t...>&, state_t const& current_state, std::ostream& out) noexcept {
  int _[]{ 0, (dump_transition<types_t>(current_state, out), 0)... };
  (void)_;
}

template <class state_machine_t, class state_t>
void dump(state_t const& current_state, std::ostream& out) noexcept {
  out << "@startuml\n\n";
  dump_transitions(typename state_machine_t::transitions{}, current_state, out);
  out << "\n@enduml\n";
}

}  // namespace detail

}  // namespace tfc::dbus::sml
