module;
#include <boost/stacktrace/stacktrace.hpp>
#include <magic_enum.hpp>

#include <csignal>
#include "tfc/utils/pragmas.hpp"

export module tfc.base;

import fmt;
import asio;

import std;
import argparse;

namespace tfc::base {

/*! Logging level*/
export enum struct log_lvl_e : int {
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

  void init(int argc, char const* const* argv, argparse::ArgumentParser program) {
    try {
      program.parse_args(argc, argv);
    } catch (const std::exception& err) {
      fmt::print(stderr, "Error parsing arguments: {}\n", err.what());
      std::exit(1);
    }
    exe_name_ = std::filesystem::path(argv[0]).filename().string();
    if (program.get<bool>("--help")) {
      fmt::print("Usage: {} [options] \n{}", exe_name_, program.help().str());
      std::exit(0);
    }
    id_ = program.get<std::string>("--id");
    stdout_ = program.get<bool>("--stdout");
    noeffect_ = program.get<bool>("--noeffect");

    auto log_level = program.get<std::string>("--log-level");
    auto enum_v = magic_enum::enum_cast<log_lvl_e>(log_level);
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

  //[[nodiscard]] auto get_map() const noexcept -> bpo::variables_map const& { return vm_; }
  [[nodiscard]] auto get_id() const -> std::string_view { return id_; }
  [[nodiscard]] auto get_exe_name() const -> std::string_view { return exe_name_; }
  [[nodiscard]] auto get_stdout() const noexcept -> bool { return stdout_; }
  [[nodiscard]] auto get_noeffect() const noexcept -> bool { return noeffect_; }
  [[nodiscard]] auto get_log_lvl() const noexcept -> log_lvl_e { return log_level_; }

private:
  options() = default;
  bool noeffect_{ false };
  bool stdout_{ false };
  std::string id_{};
  std::string exe_name_{};
  log_lvl_e log_level_{};
};

/// \brief Description of command line arguments for the program
/// \return Default description for tfc applications
export inline auto default_parser() -> argparse::ArgumentParser {
  argparse::ArgumentParser program;
  program.add_description(
      "Time For Change executable. \n"
      "Build: TODO <version>-<git hash>");

  // Dynamically fetch entries in log level
  constexpr auto lvl_values{ magic_enum::enum_entries<log_lvl_e>() };
  std::string help_text;
  std::for_each(lvl_values.begin(), lvl_values.end(), [&help_text](auto& pair) {
    help_text.append(" ");
    help_text.append(pair.second);
  });

  program.add_argument("-h", "--help").default_value(false).help("Produce this help message.");
  program.add_argument("-i","--id").default_value(std::string("def")).help("Process name used internally, max 12 characters.");
  program.add_argument("--noeffect").default_value(false).help("Process will not send any IPCs.");
  program.add_argument("--stdout").default_value(false).help("Logs displayed both in terminal and journal.");
  program.add_argument("--log-level").default_value(std::string("info")).help(fmt::format("Set log level ({})", help_text));
  return program;
}

/// \brief Function to call from main function to initialize singleton who populates the below getters.
/// \example example_base.cpp
export inline void init(int argc, char const* const* argv, argparse::ArgumentParser parser) {
  options::instance().init(argc, argv, parser);
}
export inline void init(int argc, char const* const* argv) {
  options::instance().init(argc, argv, default_parser());
}

/// \return stripped executable name
export inline auto get_exe_name() noexcept -> std::string_view {
  return options::instance().get_exe_name();
}

/// \brief default value is "def"
/// \return stripped process identification name provided by the command line argument
export inline auto get_proc_name() noexcept -> std::string_view {
  return options::instance().get_id();
}
/// \brief default value is tfc::logger::lvl_e::info
/// \return log level
export inline auto get_log_lvl() noexcept -> log_lvl_e {
  return options::instance().get_log_lvl();
}

/// \return Configuration directory path
/// default return value is /etc/tfc/
/// \note can be changed by providing environment variable CONFIGURATION_DIRECTORY
/// Refer to https://www.freedesktop.org/software/systemd/man/systemd.exec.html#%24RUNTIME_DIRECTORY
export inline auto get_config_directory() -> std::filesystem::path {
  if (auto const* config_dir{ std::getenv("CONFIGURATION_DIRECTORY") }) {
    return std::filesystem::path{ config_dir };
  }
  return std::filesystem::path{ "/etc/tfc/" };
}

/// \return <config_directory><exe_name>/<proc_name>/<filename>.<file_extension>
export inline auto make_config_file_name(std::string_view filename, std::string_view extension) -> std::filesystem::path {
  auto config_dir{ get_config_directory() };
  std::string filename_path{ filename };
  if (!extension.empty()) {
    filename_path.append(".").append(extension);
  }
  return config_dir / get_exe_name() / get_proc_name() / filename_path;
}

/// \brief supposed to be used by logger library to indicate log to terminal is enabled
export inline auto is_stdout_enabled() noexcept -> bool {
  return options::instance().get_stdout();
}

/// \brief supposed to be used by IPC layer to indicate that signals/publishers should not do anything
export inline auto is_noeffect_enabled() noexcept -> bool {
  return options::instance().get_noeffect();
}

/// \brief print stacktrace to stderr and terminate program
export [[noreturn]] inline void terminate() {
  boost::stacktrace::stacktrace const trace{};
  fmt::fprintf(stderr, "%s\n", to_string(trace).data());
  std::terminate();
}

/// \brief stop context for predefined exit signals
export inline auto exit_signals(asio::io_context& ctx) -> auto {
  //asio::signal_set signal_set{ ctx, SIGINT, SIGTERM, SIGQUIT };
  //co_await signal_set.async_wait(asio::use_awaitable_t<>());
  //fmt::print("\nShutting down gracefully.\nMay you have a pleasant remainder of your day.\n");
  ctx.stop();
}

}  // namespace tfc::base
