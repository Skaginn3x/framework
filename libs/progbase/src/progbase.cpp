#include "tfc/progbase.hpp"
#include "tfc/logger.hpp"
#include "tfc/utils/pragmas.hpp"

#include <boost/program_options.hpp>
#include <boost/stacktrace.hpp>
#include <fmt/printf.h>
#include <magic_enum.hpp>

namespace bpo = boost::program_options;

namespace tfc::base {
class options {
public:
  options(options const&) = delete;
  void operator=(options const&) = delete;

  void init(int argc, char const* const* argv, bpo::options_description const& desc) {
    vm_ = {};
    bpo::store(bpo::parse_command_line(argc, argv, desc), vm_);
    bpo::notify(vm_);
    exe_name_ = std::filesystem::path(argv[0]).filename().string();
    if (vm_["help"].as<bool>()) {
      std::stringstream out;
      desc.print(out);
      fmt::print("Usage: {} [options] \n{}", exe_name_, out.str());
      std::exit(0);
    }
    id_ = vm_["id"].as<std::string>();
    stdout_ = vm_["stdout"].as<bool>();
    noeffect_ = vm_["noeffect"].as<bool>();

    auto log_level = vm_["log-level"].as<std::string>();
    auto enum_v = magic_enum::enum_cast<tfc::logger::lvl_e>(log_level);
    if (enum_v.has_value()) {
      log_level_ = enum_v.value();
    } else {
      throw std::runtime_error(fmt::format("Invalid log_level : {}", log_level));
    }
  }

  static auto instance() -> options& {
    // clang-format off
    PRAGMA_CLANG_WARNING_PUSH_OFF(-Wexit-time-destructors)
    // clang-format on
    static options options_v;
    PRAGMA_CLANG_WARNING_POP
    return options_v;
  }

  [[nodiscard]] auto get_map() const noexcept -> bpo::variables_map const& { return vm_; }
  [[nodiscard]] auto get_id() const -> std::string_view { return id_; }
  [[nodiscard]] auto get_exe_name() const -> std::string_view { return exe_name_; }
  [[nodiscard]] auto get_stdout() const noexcept -> bool { return stdout_; }
  [[nodiscard]] auto get_noeffect() const noexcept -> bool { return noeffect_; }
  [[nodiscard]] auto get_log_lvl() const noexcept -> tfc::logger::lvl_e { return log_level_; }

private:
  options() = default;
  bool noeffect_{ false };
  bool stdout_{ false };
  std::string id_{};
  std::string exe_name_{};
  bpo::variables_map vm_{};
  tfc::logger::lvl_e log_level_{};
};

auto default_description() -> boost::program_options::options_description {
  bpo::options_description description{
    "Time For Change executable. \n"
    "Build: TODO <version>-<git hash>"
  };

  // Dynamically fetch entries in log level
  constexpr auto lvl_values{ magic_enum::enum_entries<tfc::logger::lvl_e>() };
  std::string help_text;
  std::for_each(lvl_values.begin(), lvl_values.end(), [&help_text](auto& pair) {
    help_text.append(" ");
    help_text.append(pair.second);
  });

  description.add_options()("help,h", bpo::bool_switch()->default_value(false), "Produce this help message.")(
      "id,i", bpo::value<std::string>()->default_value("def"), "Process name used internally, max 12 characters.")(
      "noeffect", bpo::bool_switch()->default_value(false), "Process will not send any IPCs.")(
      "stdout", bpo::bool_switch()->default_value(false), "Logs displayed both in terminal and journal.")(
      "log-level", bpo::value<std::string>()->default_value("info"), fmt::format("Set log level ({})", help_text).c_str());
  return description;
}

void init(int argc, char const* const* argv, bpo::options_description const& desc) {
  options::instance().init(argc, argv, desc);
}
void init(int argc, char const* const* argv) {
  options::instance().init(argc, argv, default_description());
}

auto get_exe_name() noexcept -> std::string_view {
  return options::instance().get_exe_name();
}
auto get_proc_name() noexcept -> std::string_view {
  return options::instance().get_id();
}
auto get_log_lvl() noexcept -> tfc::logger::lvl_e {
  return options::instance().get_log_lvl();
}
auto get_map() noexcept -> boost::program_options::variables_map const& {
  return options::instance().get_map();
}
auto get_root_path() -> std::filesystem::path {
  return { "/var/tfc" };
}
auto is_stdout_enabled() noexcept -> bool {
  return options::instance().get_stdout();
}
auto is_noeffect_enabled() noexcept -> bool {
  return options::instance().get_noeffect();
}

void terminate() {
  boost::stacktrace::stacktrace const trace{};
  fmt::fprintf(stderr, "%s\n", to_string(trace).data());
  std::terminate();
}


}  // namespace tfc::base
