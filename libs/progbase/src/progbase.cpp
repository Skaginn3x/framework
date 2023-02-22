#include "tfc/progbase.hpp"

#include <filesystem>

#include <boost/program_options.hpp>

namespace bpo = boost::program_options;

namespace tfc::base {

class options {
public:
  options(options const &) = delete;
  void operator=(options const &) = delete;

  void init(int argc, char const *const *argv, bpo::options_description const &desc) {
    vm_ = {};
    bpo::store(bpo::parse_command_line(argc, argv, desc), vm_);
    bpo::notify(vm_);
    exe_name_ = std::filesystem::path(argv[0]).filename().string();
    id_ = vm_["id"].as<std::string>();
    stdout_ = vm_["stdout"].as<bool>();
    noeffect_ = vm_["noeffect"].as<bool>();
  }

  static auto instance() -> options & {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
    static options options_v;
#pragma clang diagnostic pop
    return options_v;
  }

  [[nodiscard]] auto get_map() const noexcept -> bpo::variables_map const & { return vm_; }
  [[nodiscard]] auto get_id() const -> std::string_view { return id_; }
  [[nodiscard]] auto get_exe_name() const -> std::string_view { return exe_name_; }
  [[nodiscard]] auto get_stdout() const noexcept -> bool { return stdout_; }
  [[nodiscard]] auto get_noeffect() const noexcept -> bool { return noeffect_; }

private:
  options() = default;
  bool noeffect_{false};
  bool stdout_{false};
  std::string id_{};
  std::string exe_name_{};
  bpo::variables_map vm_{};
};

auto default_description() -> boost::program_options::options_description {
  bpo::options_description description{"Time For Change executable. \n"
                                       "Build: TODO <version>-<git hash>"};
  description.add_options()("help,h", bpo::value<std::string>(), "Produce this help message.")(
      "id,i", bpo::value<std::string>()->default_value("def"), "Process name used internally, max 12 characters.")(
      "noeffect", bpo::value<bool>()->default_value(false), "Process will not send any IPCs.")(
      "stdout", bpo::value<bool>()->default_value(false), "Logs displayed both in terminal and journal.");
  return description;
}

void init(int argc, char const *const *argv, bpo::options_description const &desc) {
  options::instance().init(argc, argv, desc);
}

auto get_exe_name() noexcept -> std::string_view { return options::instance().get_exe_name(); }
auto get_proc_name() noexcept -> std::string_view { return options::instance().get_id(); }
auto get_map() noexcept -> boost::program_options::variables_map const & { return options::instance().get_map(); }
auto get_root_path() -> std::filesystem::path { return {"/var/tfc"}; }
auto is_stdout_enabled() noexcept -> bool { return options::instance().get_stdout(); }
auto is_noeffect_enabled() noexcept -> bool { return options::instance().get_noeffect(); }

} // namespace tfc::base
