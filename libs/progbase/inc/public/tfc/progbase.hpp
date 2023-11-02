#pragma once

#include <filesystem>
#include <optional>
#include <string_view>

namespace argparse{
class ArgumentParser;
}  // namespace program_options
namespace boost {
namespace asio {
class any_io_executor;
template <typename return_t, typename executor_t>
class awaitable;
class io_context;
}  // namespace asio
}  // namespace boost

namespace tfc::logger {
enum struct lvl_e : int;
}

namespace tfc::base {

/// \brief Description of command line arguments for the program
/// \return Default description for tfc applications
[[nodiscard]] auto default_parser() -> argparse::ArgumentParser;

/// \brief Function to call from main function to initialize singleton who populates the below getters.
/// \example example_base.cpp
void init(int argc, char const* const* argv, argparse::ArgumentParser const& parser);
void init(int argc, char const* const* argv);

/// \return stripped executable name
[[nodiscard]] auto get_exe_name() noexcept -> std::string_view;

/// \brief default value is "def"
/// \return stripped process identification name provided by the command line argument
[[nodiscard]] auto get_proc_name() noexcept -> std::string_view;

/// \brief default value is tfc::logger::lvl_e::info
/// \return log level
[[nodiscard]] auto get_log_lvl() noexcept -> tfc::logger::lvl_e;

/// \return boost variables map if needed to get custom parameters from description
//[[nodiscard]] auto get_map() noexcept -> boost::program_options::variables_map const&;

/// \return Configuration directory path
/// default return value is /etc/tfc/
/// \note can be changed by providing environment variable CONFIGURATION_DIRECTORY
/// Refer to https://www.freedesktop.org/software/systemd/man/systemd.exec.html#%24RUNTIME_DIRECTORY
[[nodiscard]] auto get_config_directory() -> std::filesystem::path;

/// \return <config_directory><exe_name>/<proc_name>/<filename>.<file_extension>
[[nodiscard]] auto make_config_file_name(std::string_view filename, std::string_view extension) -> std::filesystem::path;

/// \brief supposed to be used by logger library to indicate log to terminal is enabled
[[nodiscard]] auto is_stdout_enabled() noexcept -> bool;

/// \brief supposed to be used by IPC layer to indicate that signals/publishers should not do anything
[[nodiscard]] auto is_noeffect_enabled() noexcept -> bool;

/// \brief print stacktrace to stderr and terminate program
[[noreturn]] void terminate();

/// \brief stop context for predefined exit signals
auto exit_signals(boost::asio::io_context&) -> boost::asio::awaitable<void, boost::asio::any_io_executor>;

}  // namespace tfc::base
