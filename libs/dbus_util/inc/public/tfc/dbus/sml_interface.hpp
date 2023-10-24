#pragma once

#include <functional>
#include <memory>
#include <sstream>
#include <string>

#include <boost/sml.hpp>

#include <tfc/dbus/sdbusplus_fwd.hpp>
#include <tfc/dbus/string_maker.hpp>

namespace tfc::dbus::sml {

namespace tags {
static constexpr std::string_view sub_path{ "StateMachines" };
static constexpr std::string_view path{ const_dbus_path<sub_path> };
}

struct interface_impl {
  explicit interface_impl(std::shared_ptr<sdbusplus::asio::dbus_interface>);
  void on_state_change(std::string_view source_state, std::string_view destination_state, std::string_view event);
  void dot_format(std::string_view state_machine);

  std::string source_state_{};
  std::string destination_state_{};
  std::string event_{};
  std::string state_machine_dot_formatted_{};
  std::shared_ptr<sdbusplus::asio::dbus_interface> dbus_interface_{};
};

namespace detail {
template <class state_machine_t, class state_t>
void dump(std::ostream& out) noexcept;
}

struct interface {
  explicit interface(std::shared_ptr<sdbusplus::asio::dbus_interface> interface) : impl_{ std::move(interface) } {}

//  template <template <typename first_t, typename second_t> typename event_t>
//  struct extract_event_type {
//    using type = second_t;
//  };

  template <class state_machine_t, class event_t>
  void log_process_event(const event_t& /*event*/) {  // NOLINT(readability-identifier-naming)
    last_event_ = boost::sml::aux::get_type_name<event_t>();
  }

  template <class state_machine_t, class guard_t, class event_t>
  void log_guard(const guard_t& /*guard*/, const event_t& /*event*/, bool result) {}

  template <class state_machine_t, class action_t, class event_t>
  void log_action(const action_t& /*action*/, const event_t& /*event*/) {}

  template <class state_machine_t, class source_state_t, class destination_state_t>
  void log_state_change(const source_state_t& src, const destination_state_t& dst) {
    impl_.on_state_change(src.c_str(), dst.c_str(), last_event_);
//    std::stringstream iss{};
//    detail::dump<state_machine_t, destination_state_t>(iss);
//    impl_.dot_format(iss.str());
  }

  interface_impl impl_;
  std::string last_event_{};
};

namespace detail {
// modified version of https://boost-ext.github.io/sml/examples.html
// added color to current state
template <class T>
void dump_transition(std::ostream& out) noexcept {
  auto src_state = std::string{ boost::sml::aux::string<typename T::src_state>{}.c_str() };
  auto dst_state = std::string{ boost::sml::aux::string<typename T::dst_state>{}.c_str() };
  if (dst_state == "X") {
    dst_state = "[*]";
  }

  if (T::initial) {
    out << "[*] --> " << src_state << "\n";
  }

  const auto has_event = !boost::sml::aux::is_same<typename T::event, boost::sml::anonymous>::value;
  const auto has_guard = !boost::sml::aux::is_same<typename T::guard, boost::sml::front::always>::value;
  const auto has_action = !boost::sml::aux::is_same<typename T::action, boost::sml::front::none>::value;

  const auto is_entry =
      boost::sml::aux::is_same<typename T::event, boost::sml::back::on_entry<boost::sml::_, boost::sml::_>>::value;
  const auto is_exit =
      boost::sml::aux::is_same<typename T::event, boost::sml::back::on_exit<boost::sml::_, boost::sml::_>>::value;

  if (is_entry || is_exit) {
    out << src_state;
  } else {  // state to state transition
    out << src_state << " --> " << dst_state;
  }

  if (has_event || has_guard || has_action) {
    out << " :";
  }

  if (has_event) {
    auto event = std::string(boost::sml::aux::get_type_name<typename T::event>());
    if (is_entry) {
      event = "entry";
    } else if (is_exit) {
      event = "exit";
    }
    out << " " << event;
  }

  if (has_guard) {
    out << " [" << boost::sml::aux::get_type_name<typename T::guard::type>() << "]";
  }

  if (has_action) {
    out << " / " << boost::sml::aux::get_type_name<typename T::action::type>();
  }

  out << "\n";
}

template <template <class...> class T, class... Ts>
void dump_transitions(const T<Ts...>&, std::ostream& out) noexcept {
  int _[]{ 0, (dump_transition<Ts>(out), 0)... };
  (void)_;
}

template <class state_machine_t, class state_t>
void dump(std::ostream& out) noexcept {
  out << "@startuml\n\n";
  dump_transitions(typename state_machine_t::transitions{}, out);
  out << "\n@enduml\n";
}

}  // namespace detail

}  // namespace tfc::dbus::sml
