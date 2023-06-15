// Standard
#include <memory>

// Third party
#include <fmt/core.h>
#include <boost/sml.hpp>
#include <tfc/logger.hpp>

namespace tfc::logger {

/**
 * @brief Boost state machine library (SML) logger with tfc::logger instance.
 * */
struct sml_logger {
  /**
   * @brief log event that happens in the state machine to the logger
   * */
  template <class SM, class TEvent>
  void log_process_event(const TEvent& /*event*/) {  // NOLINT(readability-identifier-naming)
    logger_->log<tfc::logger::lvl_e::info>("[{}][process_event] {}\n", boost::sml::aux::get_type_name<SM>(),
                                           boost::sml::aux::get_type_name<TEvent>());
  }

  /**
   * @brief log guard that is checked in the state machine, outputs whether the guard is OK or Rejected
   * */
  template <class SM, class TGuard, class TEvent>
  void log_guard(const TGuard& /*guard*/,
                 const TEvent& /*event*/,
                 bool result) {  // NOLINT(readability-identifier-naming)
    logger_->log<tfc::logger::lvl_e::info>("[{}][guard] {} {} {}\n", boost::sml::aux::get_type_name<SM>(),
                                           boost::sml::aux::get_type_name<TGuard>(),
                                           boost::sml::aux::get_type_name<TEvent>(), (result ? "[OK]" : "[Reject]"));
  }

  /**
   * @brief log action that is taken in the state machine
   * */
  template <class SM, class TAction, class TEvent>
  void log_action(const TAction& /*action*/, const TEvent& /*event*/) {  // NOLINT(readability-identifier-naming)
    logger_->log<tfc::logger::lvl_e::info>("[{}][action] {} {}\n", boost::sml::aux::get_type_name<SM>(),
                                           boost::sml::aux::get_type_name<TAction>(),
                                           boost::sml::aux::get_type_name<TEvent>());
  }

  /**
   * @brief log state change that happens in the state machine
   * */
  template <class SM, class TSrcState, class TDstState>
  void log_state_change(const TSrcState& src, const TDstState& dst) {  // NOLINT(readability-identifier-naming)
    logger_->log<tfc::logger::lvl_e::info>("[{}][transition] {} -> {}\n", boost::sml::aux::get_type_name<SM>(), src.c_str(),
                                           dst.c_str());
  }

private:
  std::shared_ptr<tfc::logger::logger> logger_{ std::make_shared<tfc::logger::logger>("sml") };
};
}  // namespace tfc::logger
