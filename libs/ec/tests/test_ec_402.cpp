#include <iostream>
#include "tfc/cia/402.hpp"

namespace sml = boost::sml;

template <class T>
void dump_transition(std::ostream& out) noexcept {
  auto src_state = std::string{sml::aux::string<typename T::src_state>{}.c_str()};
  auto dst_state = std::string{sml::aux::string<typename T::dst_state>{}.c_str()};
  if (dst_state == "X") {
    dst_state = "[*]";
  }

  if (T::initial) {
    out << "[*] --> " << src_state << "\n";
  }

  const auto has_event = !sml::aux::is_same<typename T::event, sml::anonymous>::value;
  const auto has_guard = !sml::aux::is_same<typename T::guard, sml::front::always>::value;
  const auto has_action = !sml::aux::is_same<typename T::action, sml::front::none>::value;

  const auto is_entry = sml::aux::is_same<typename T::event, sml::back::on_entry<sml::_, sml::_>>::value;
  const auto is_exit = sml::aux::is_same<typename T::event, sml::back::on_exit<sml::_, sml::_>>::value;

  // entry / exit entry
  if (is_entry || is_exit) {
    out << src_state;
  } else {  // state to state transition
    out << src_state << " --> " << dst_state;
  }

  if (has_event || has_guard || has_action) {
    out << " :";
  }

  if (has_event) {
    // handle 'on_entry' and 'on_exit' per plant-uml syntax
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
  int _[]{0, (dump_transition<Ts>(out), 0)...};
  (void)_;
}

template <class SM>
void dump(const SM&, std::ostream& out) noexcept {
  out << "@startuml\n\n";
  dump_transitions(typename SM::transitions{}, out);
  out << "\n@enduml\n";
}

// int main() {
//   sml::sm<plant_uml> sm;
//   dump(sm, std::cout);
// }
int main(){
  sml::sm<tfc::ec::cia::state_machine> sm;
  dump(sm, std::cout);
}
