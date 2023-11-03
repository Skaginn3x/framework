#include "tfc/progbase.hpp"
#include "tfc/logger.hpp"
#include "tfc/utils/pragmas.hpp"

#include <fmt/printf.h>
#include <boost/asio.hpp>
#include <boost/stacktrace.hpp>
#include <magic_enum.hpp>

import std;
import argparse;

namespace asio = boost::asio;

namespace tfc::base {
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

  //[[nodiscard]] auto get_map() const noexcept -> bpo::variables_map const& { return vm_; }
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
  tfc::logger::lvl_e log_level_{};
};

auto default_parser() -> argparse::ArgumentParser {
  argparse::ArgumentParser program;
  program.add_description(
      "Time For Change executable. \n"
      "Build: TODO <version>-<git hash>");

  // Dynamically fetch entries in log level
  constexpr auto lvl_values{ magic_enum::enum_entries<tfc::logger::lvl_e>() };
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

void init(int argc, char const* const* argv, argparse::ArgumentParser parser) {
  options::instance().init(argc, argv, parser);
}

void init(int argc, char const* const* argv) {
  options::instance().init(argc, argv, default_parser());
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
// auto get_map() noexcept -> boost::program_options::variables_map const& {
//   return options::instance().get_map();
// }

auto get_config_directory() -> std::filesystem::path {
  if (auto const* config_dir{ std::getenv("CONFIGURATION_DIRECTORY") }) {
    return std::filesystem::path{ config_dir };
  }
  return std::filesystem::path{ "/etc/tfc/" };
}
auto make_config_file_name(std::string_view filename, std::string_view extension) -> std::filesystem::path {
  auto config_dir{ get_config_directory() };
  std::string filename_path{ filename };
  if (!extension.empty()) {
    filename_path.append(".").append(extension);
  }
  return config_dir / get_exe_name() / get_proc_name() / filename_path;
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

auto exit_signals(asio::io_context& ctx) -> asio::awaitable<void> {
  auto executor = co_await asio::this_coro::executor;
  asio::signal_set signal_set{ executor, SIGINT, SIGTERM, SIGQUIT };
  co_await signal_set.async_wait(asio::use_awaitable);
  fmt::print("\nShutting down gracefully.\nMay you have a pleasant remainder of your day.\n");
  ctx.stop();
}

}  // namespace tfc::base
