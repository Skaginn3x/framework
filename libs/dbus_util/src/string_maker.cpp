#include <fmt/format.h>

#include <tfc/dbus/exception.hpp>
#include <tfc/dbus/string_maker.hpp>
#include <tfc/progbase.hpp>

namespace tfc::dbus {

auto make_dbus_name(std::string_view input_name) -> std::string {
  auto return_value{ detail::make<detail::dbus_name_prefix>(input_name) };
  if (return_value.contains('-')) {
    throw exception::invalid_name{ "{} contains illegal dbus character '-'", input_name };
  }
  if (return_value.contains("..")) {
    throw exception::invalid_name{ "{} contains illegal dbus characters '..'", input_name };
  }
  if (return_value.contains("/")) {
    throw exception::invalid_name{ "{} contains illegal dbus characters '/'", input_name };
  }
  std::smatch match_results{};
  static constexpr std::string_view match_dots_preceding_number{ "(\\.)+(?=\\d)" };
  if (std::regex_search(return_value, match_results, std::regex{ match_dots_preceding_number.data() })) {
    throw exception::invalid_name{ "{} contains dots preceding number", input_name };
  }
  return return_value;
}

auto make_dbus_path(std::string_view input_name) -> std::string {
  auto return_value{ detail::make<detail::dbus_path_prefix>(input_name) };
  if (return_value.contains('-')) {
    throw exception::invalid_name{ "{} contains illegal dbus character '-'", input_name };
  }
  if (return_value.contains("//")) {
    throw exception::invalid_name{ "{} contains illegal dbus characters \"//\"", input_name };
  }
  return return_value;
}

auto make_dbus_process_name() -> std::string {
  auto postfix{ fmt::format("tfc.{}.{}", base::get_exe_name(), base::get_proc_name()) };
  return make_dbus_name(postfix);
}

auto strip_dbus_name(std::string_view input_name) -> std::string {
  if (input_name.starts_with(detail::dbus_name_prefix)) {
    return std::string{ input_name.substr(detail::dbus_name_prefix.size()) };
  }
  return std::string{ input_name };
}

}  // namespace tfc::dbus
