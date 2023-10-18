module;

#include <filesystem>
#include <optional>
#include <ranges>
#include <string_view>

#include <fmt/printf.h>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/stacktrace.hpp>
#include <magic_enum.hpp>
#include <tfc/utils/pragmas.hpp>

export module tfc.progbase;

namespace tfc::base {

namespace bpo = boost::program_options;
namespace asio = boost::asio;

/*! Logging level*/
export enum struct lvl_e : int {
  trace = 0,
  debug = 1,
  info = 2,
  warn = 3,
  error = 4,
  critical = 5,
  off = 6, /*! Log regardless of set logging level*/
};

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
    auto enum_v = magic_enum::enum_cast<tfc::base::lvl_e>(log_level);
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
  [[nodiscard]] auto get_log_lvl() const noexcept -> tfc::base::lvl_e { return log_level_; }

private:
  options() = default;
  bool noeffect_{ false };
  bool stdout_{ false };
  std::string id_{};
  std::string exe_name_{};
  bpo::variables_map vm_{};
  lvl_e log_level_{};
};

/// \brief Description of command line arguments for the program
/// \return Default description for tfc applications
export [[nodiscard]] inline auto default_description() -> boost::program_options::options_description {
  bpo::options_description description{
    "Time For Change executable. \n"
    "Build: TODO <version>-<git hash>"
  };

  // Dynamically fetch entries in log level
  constexpr auto lvl_values{ magic_enum::enum_entries<tfc::base::lvl_e>() };
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

/// \brief Function to call from main function to initialize singleton who populates the below getters.
/// \example example_base.cpp
export inline void init(int argc, char const* const* argv, boost::program_options::options_description const& desc) {
  options::instance().init(argc, argv, desc);
}
export inline void init(int argc, char const* const* argv) {
  options::instance().init(argc, argv, default_description());
}

/// \return stripped executable name
export [[nodiscard]] inline auto get_exe_name() noexcept -> std::string_view {
  return options::instance().get_exe_name();
}

/// \brief default value is "def"
/// \return stripped process identification name provided by the command line argument
export [[nodiscard]] inline auto get_proc_name() noexcept -> std::string_view {
  return options::instance().get_id();
}

/// \brief default value is tfc::logger::lvl_e::info
/// \return log level
export [[nodiscard]] inline auto get_log_lvl() noexcept -> tfc::base::lvl_e {
  return options::instance().get_log_lvl();
}

/// \return boost variables map if needed to get custom parameters from description
export [[nodiscard]] inline auto get_map() noexcept -> boost::program_options::variables_map const& {
  return options::instance().get_map();
}

/// \return Configuration directory path
/// default return value is /etc/tfc/
/// \note can be changed by providing environment variable CONFIGURATION_DIRECTORY
/// Refer to https://www.freedesktop.org/software/systemd/man/systemd.exec.html#%24RUNTIME_DIRECTORY
export [[nodiscard]] inline auto get_config_directory() -> std::filesystem::path {
  if (auto const* config_dir{ std::getenv("CONFIGURATION_DIRECTORY") }) {
    return std::filesystem::path{ config_dir };
  }
  return std::filesystem::path{ "/etc/tfc/" };
}

/// \return <config_directory><exe_name>/<proc_name>/<filename>.<file_extension>
export [[nodiscard]] inline auto make_config_file_name(std::string_view filename, std::string_view extension) -> std::filesystem::path {
  auto config_dir{ get_config_directory() };
  std::filesystem::path filename_path{ filename };
  if (!extension.empty()) {
    filename_path.replace_extension(extension);
  }
  return config_dir / get_exe_name() / get_proc_name() / filename_path;
}

/// \brief supposed to be used by logger library to indicate log to terminal is enabled
export [[nodiscard]] inline auto is_stdout_enabled() noexcept -> bool {
  return options::instance().get_stdout();
}

/// \brief supposed to be used by IPC layer to indicate that signals/publishers should not do anything
export [[nodiscard]] inline auto is_noeffect_enabled() noexcept -> bool {
  return options::instance().get_noeffect();
}

/// \brief print stacktrace to stderr and terminate program
export [[noreturn]] inline void terminate() {
  boost::stacktrace::stacktrace const trace{};
  fmt::fprintf(stderr, "%s\n", to_string(trace).data());
  std::terminate();
}

/// \brief stop context for predefined exit signals
export inline auto exit_signals(boost::asio::io_context& ctx) -> boost::asio::awaitable<void, boost::asio::any_io_executor> {
  auto executor = co_await asio::this_coro::executor;
  asio::signal_set signal_set{ executor, SIGINT, SIGTERM, SIGQUIT };
  co_await signal_set.async_wait(asio::use_awaitable);
  fmt::print("\nShutting down gracefully.\nMay you have a pleasant remainder of your day.\n");
  ctx.stop();
}

}  // namespace tfc::base
