#pragma once

#include <string_view>
#include <optional>
#include <filesystem>

namespace boost::program_options {
class options_description;
class variables_map;
} // namespace boost::program_options

namespace tfc::logger {
enum struct lvl_e : int;
}

namespace tfc::base {

/// \brief Description of command line arguments for the program
/// \return Default description for tfc applications
[[maybe_unused, nodiscard]] auto default_description() -> boost::program_options::options_description;

/// \brief Function to call from main function to initialize singleton who populates the below getters.
/// \example example_base.cpp
[[maybe_unused]] void init(int argc, char const *const *argv, boost::program_options::options_description const &desc);

/// \return stripped executable name
[[maybe_unused, nodiscard]] auto get_exe_name() noexcept -> std::string_view;

/// \brief default value is "def"
/// \return stripped process identification name provided by the command line argument
[[maybe_unused, nodiscard]] auto get_proc_name() noexcept -> std::string_view;

/// \brief default value is tfc::logger::lvl_e::info
/// \return log level
[[maybe_unused, nodiscard]] auto get_log_lvl() noexcept -> tfc::logger::lvl_e;

/// \return boost variables map if needed to get custom parameters from description
[[maybe_unused, nodiscard]] auto get_map() noexcept -> boost::program_options::variables_map const &;

/// \return path to root of application files
[[maybe_unused, nodiscard]] auto get_root_path() -> std::filesystem::path;

/// \brief supposed to be used by logger library to indicate log to terminal is enabled
[[maybe_unused, nodiscard]] auto is_stdout_enabled() noexcept -> bool;

/// \brief supposed to be used by IPC layer to indicate that signals/publishers should not do anything
[[maybe_unused, nodiscard]] auto is_noeffect_enabled() noexcept -> bool;

} // namespace tfc::base
