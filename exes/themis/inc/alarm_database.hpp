#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <map>
#include <optional>
#include <string>

#include <fmt/args.h>
#include <fmt/format.h>
#include <openssl/sha.h>
#include <sqlite_modern_cpp.h>
#include <sqlite_modern_cpp/log.h>

#include <tfc/dbus/exception.hpp>
#include <tfc/logger.hpp>
#include <tfc/progbase.hpp>
#include <tfc/snitch/common.hpp>

namespace tfc::themis {

using enum tfc::snitch::level_e;
using tfc::snitch::api::time_point;
using dbus_error = tfc::dbus::exception::runtime;

class error_log {
public:
  error_log() : logger_("alarm_database") {
    sqlite::error_log([this](const std::exception& e) { logger_.error("{}", e.what()); });
  }

private:
  tfc::logger::logger logger_;
};

inline auto config_file_name_populate_dir() -> std::string {
  auto const file{ base::make_config_file_name(base::get_exe_name(), "db") };
  std::filesystem::create_directories(file.parent_path());
  return file.string();
}
class alarm_database {
public:
  explicit alarm_database(bool in_memory = false) : db_(in_memory ? ":memory:" : config_file_name_populate_dir()) {
    // Set foreign key enforcement to modern standards
    db_ << "PRAGMA foreign_keys = ON;";
    db_ << R"(
CREATE TABLE IF NOT EXISTS Alarms(
  alarm_id INTEGER PRIMARY KEY,
  tfc_id TEXT NOT NULL,
  sha1sum TEXT NOT NULL,
  alarm_level INTEGER NOT NULL,
  alarm_latching BOOLEAN NOT NULL,
  inserted_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  registered_at LONG INTEGER NOT NULL,
  UNIQUE(tfc_id, sha1sum) ON CONFLICT IGNORE
);)";
    db_ << R"(
CREATE TABLE IF NOT EXISTS AlarmTranslations(
  sha1sum TEXT NOT NULL,
  alarm_id INTEGER NOT NULL,
  locale TEXT NOT NULL,
  details TEXT NOT NULL,
  description TEXT NOT NULL,
  inserted_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY(sha1sum, locale) ON CONFLICT REPLACE
  FOREIGN KEY(alarm_id) REFERENCES Alarms(alarm_id)
);
)";
    // SQLITE is creating an automatic index on queries. Create it manually
    // Mostly to avoid the log messages
    db_ << "CREATE INDEX IF NOT EXISTS locale_idx ON AlarmTranslations(locale);";
    // Timestamp stored as long integer to get millisecond precision
    // And because sqlite does not offer a native timestamp type.
    db_ << R"(
CREATE TABLE IF NOT EXISTS AlarmActivations(
  activation_id INTEGER PRIMARY KEY,
  alarm_id INTEGER NOT NULL,
  activation_time LONG INTEGER NOT NULL,
  reset_time LONG INTEGER,
  activation_level SHORT INTEGER NOT NULL,
  inserted_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY(alarm_id) REFERENCES Alarms(alarm_id)
);
)";
    db_ << R"(
CREATE TABLE IF NOT EXISTS AlarmVariables(
  activation_id INTEGER NOT NULL,
  variable_key TEXT NOT NULL,
  variable_value TEXT NOT NULL,
  FOREIGN KEY(activation_id) REFERENCES AlarmActivations(activation_id)
);
)";
  }
  /**
   * @brief Register an alarm in the database
   * @param tfc_id The TFC ID of the alarm
   * @param description The message of the alarm
   * @param details The short message of the alarm
   * @param latching Whether the alarm is latching
   * @param alarm_level The alarm level
   * @return The alarm ID
   */
  [[nodiscard]] auto register_alarm_en(std::string_view tfc_id,
                                       std::string_view description,
                                       std::string_view details,
                                       bool latching,
                                       tfc::snitch::level_e alarm_level,
                                       std::optional<time_point> registered_at = std::nullopt) -> snitch::api::alarm_id_t {
    std::string sha1_ascii = get_sha1(fmt::format("{}{}", description, details));
    snitch::api::alarm_id_t alarm_id = 0;
    auto ms_count_registered_at = milliseconds_since_epoch(registered_at);
    try {
      db_ << "BEGIN;";
      db_ << fmt::format(
          "INSERT INTO Alarms(tfc_id, sha1sum, alarm_level, alarm_latching, registered_at) VALUES('{}','{}',{}, {}, {}) ON "
          "CONFLICT (tfc_id, sha1sum) DO UPDATE SET registered_at={};",
          tfc_id, sha1_ascii, static_cast<std::uint8_t>(alarm_level), latching ? 1 : 0, ms_count_registered_at,
          ms_count_registered_at);
      auto insert_id = db_.last_insert_rowid();
      if (insert_id < 0) {
        throw dbus_error("Failed to insert alarm into database");
      }
      alarm_id = static_cast<snitch::api::alarm_id_t>(insert_id);
      add_alarm_translation(alarm_id, "en", description, details);

      // Reset the alarm if high on register
      if (is_alarm_active(alarm_id)) {
        reset_alarm(alarm_id);
      }
      db_ << "COMMIT;";
    } catch (std::exception& e) {
      // Rollback the transaction and rethrow
      db_ << "ROLLBACK;";
      throw e;
    }
    return alarm_id;
  }

  [[nodiscard]] auto list_alarms() -> std::vector<tfc::snitch::api::alarm> {
    // std::map to maintain alarm order.
    std::map<std::uint64_t, tfc::snitch::api::alarm> alarms;
    std::string query = R"(
SELECT
  Alarms.alarm_id,
  Alarms.tfc_id,
  Alarms.sha1sum,
  alarm_level,
  Alarms.alarm_latching,
  Alarms.registered_at,
  AlarmTranslations.locale,
  AlarmTranslations.description,
  AlarmTranslations.details
FROM Alarms
LEFT JOIN AlarmTranslations
ON Alarms.sha1sum = AlarmTranslations.sha1sum;
)";
    // Accept the second table paramters as unique ptr's as the values can be null, and we want to preserve that
    db_ << query >> [&](snitch::api::alarm_id_t alarm_id, std::string tfc_id, std::string sha1sum, std::uint8_t alarm_level,
                        bool alarm_latching, std::optional<int64_t> registered_at, std::optional<std::string> locale,
                        std::optional<std::string> translated_description, std::optional<std::string> translated_details) {
      auto iterator = alarms.find(alarm_id);
      if (iterator == alarms.end()) {
        std::optional<time_point> final_registered_at = std::nullopt;
        if (registered_at.has_value()) {
          final_registered_at = timepoint_from_milliseconds(registered_at.value());
        }
        alarms.insert({ alarm_id, tfc::snitch::api::alarm{ alarm_id,
                                                           tfc_id,
                                                           sha1sum,
                                                           static_cast<tfc::snitch::level_e>(alarm_level),
                                                           alarm_latching,
                                                           final_registered_at,
                                                           {} } });
      }
      if (locale.has_value() && translated_details.has_value() && translated_description.has_value()) {
        alarms[alarm_id].translations.insert(
            { locale.value(), { translated_description.value(), translated_details.value() } });
      }
    };
    std::vector<tfc::snitch::api::alarm> alarm_list;
    for (auto& [_, alarm] : alarms) {
      alarm_list.push_back(alarm);
    }
    return alarm_list;
  }

  auto add_alarm_translation(snitch::api::alarm_id_t alarm_id,
                             std::string_view locale,
                             std::string_view description,
                             std::string_view details) -> void {
    auto query = fmt::format(
        "INSERT INTO AlarmTranslations(sha1sum, alarm_id, locale, description, details) SELECT DISTINCT sha1sum, {}, "
        "'{}','{}','{}' FROM Alarms where alarm_id = {}",
        alarm_id, locale, description, details, alarm_id);
    db_ << query;
  }

  [[nodiscard]] auto is_alarm_active(snitch::api::alarm_id_t alarm_id) -> bool {
    std::int64_t count = 0;
    db_ << fmt::format("SELECT COUNT(*) FROM AlarmActivations WHERE alarm_id = {} AND activation_level = 1;", alarm_id) >>
        [&](std::int64_t c) { count = c; };
    return count > 0;
  }

  [[nodiscard]] auto count_active_alarms() -> std::int64_t {
    std::int64_t count = 0;
    db_ << fmt::format("SELECT COUNT(*) FROM AlarmActivations WHERE activation_level = 1;") >>
        [&](std::int64_t c) { count = c; };
    return count;
  }

  [[nodiscard]] auto is_activation_high(snitch::api::alarm_id_t activation_id) -> bool {
    bool active = false;
    db_ << fmt::format("SELECT activation_level FROM AlarmActivations WHERE activation_id = {} AND activation_level = 1;",
                       activation_id) >>
        [&](bool a) { active = a; };
    return active;
  }

  [[nodiscard]] auto active_alarm_count() -> std::uint64_t {
    std::uint64_t count = 0;
    db_ << "SELECT COUNT(*) FROM AlarmActivations WHERE activation_level = 1;" >> [&](std::uint64_t c) { count = c; };
    return count;
  }

  [[nodiscard]] auto is_some_alarm_active() -> bool { return active_alarm_count() > 0; }

  /**
   * @brief Set an alarm in the database
   * @param alarm_id the id of the alarm
   * @param variables the variables for this activation
   * @param tp an optional timepoint
   * @return the activation_id
   */
  [[nodiscard]] auto set_alarm(snitch::api::alarm_id_t alarm_id,
                               const std::unordered_map<std::string, std::string>& variables,
                               std::optional<tfc::snitch::api::time_point> tp = {}) -> std::uint64_t {
    if (is_alarm_active(alarm_id)) {
      throw dbus_error("Alarm is already active");
    }
    db_ << "BEGIN;";
    std::uint64_t activation_id;
    try {
      db_ << fmt::format("INSERT INTO AlarmActivations(alarm_id, activation_time, activation_level) VALUES({},{},{})",
                         alarm_id, milliseconds_since_epoch(tp),
                         static_cast<std::int16_t>(tfc::snitch::api::active_e::active));
      activation_id = static_cast<std::uint64_t>(db_.last_insert_rowid());

      for (auto& [key, value] : variables) {
        db_ << fmt::format("INSERT INTO AlarmVariables(activation_id, variable_key, variable_value) VALUES({},'{}','{}');",
                           activation_id, key, value);
      }
    } catch (std::exception& e) {
      db_ << "ROLLBACK;";
      throw e;
    }
    db_ << "COMMIT;";
    return activation_id;
  }
  auto reset_alarm(snitch::api::alarm_id_t activation_id, std::optional<tfc::snitch::api::time_point> tp = {}) -> void {
    if (!is_activation_high(activation_id)) {
      throw dbus_error("Cannot reset an inactive activation");
    }
    db_ << fmt::format("UPDATE AlarmActivations SET activation_level = {}, reset_time = {} WHERE activation_id = {};",
                       static_cast<std::int16_t>(tfc::snitch::api::active_e::inactive), milliseconds_since_epoch(tp),
                       activation_id);
  }

  [[nodiscard]] auto list_activations(std::string_view locale,
                                      std::uint64_t start_count,
                                      std::uint64_t count,
                                      tfc::snitch::level_e level,
                                      tfc::snitch::api::active_e active,
                                      std::optional<tfc::snitch::api::time_point> start,
                                      std::optional<tfc::snitch::api::time_point> end)
      -> std::vector<tfc::snitch::api::activation> {
    std::string populated_query = fmt::format(R"(SELECT
  activation_id,
  Alarms.alarm_id,
  activation_time,
  reset_time,
  activation_level,
  primary_text.details,
  primary_text.description,
  backup_text.details,
  backup_text.description,
  Alarms.alarm_latching,
  Alarms.alarm_level
FROM AlarmActivations
JOIN Alarms on (Alarms.alarm_id = AlarmActivations.alarm_id)
-- Join the table twice, if the primary text is not populated. fallback to backup.
-- Joining the table twice over has the added benefit of not having to use a subquery.
-- and a result for a single Activation will always be a single row.
LEFT OUTER JOIN AlarmTranslations as primary_text on (Alarms.alarm_id = primary_text.alarm_id and primary_text.locale = '{}')
LEFT OUTER JOIN AlarmTranslations as backup_text on (Alarms.alarm_id = backup_text.alarm_id and backup_text.locale = 'en')
WHERE activation_time >= {} AND activation_time <= {})",
                                              locale, milliseconds_since_epoch(start), milliseconds_since_epoch(end));
    if (level != tfc::snitch::level_e::all) {
      populated_query += fmt::format(" AND alarm_level = {}", static_cast<std::uint8_t>(level));
    }
    if (active != tfc::snitch::api::active_e::all) {
      populated_query += fmt::format(" AND activation_level = {}", static_cast<std::uint8_t>(active));
    }
    populated_query += fmt::format(" LIMIT {} OFFSET {};", count, start_count);

    std::vector<tfc::snitch::api::activation> activations;

    db_ << populated_query >> [&](std::uint64_t activation_id, snitch::api::alarm_id_t alarm_id,
                                  std::int64_t activation_time, std::optional<std::int64_t> reset_time,
                                  std::int64_t activation_level, std::optional<std::string> primary_details,
                                  std::optional<std::string> primary_description, std::optional<std::string> backup_details,
                                  std::optional<std::string> backup_description, bool alarm_latching,
                                  std::int8_t alarm_level) {
      if (!backup_description.has_value() || !backup_details.has_value()) {
        throw dbus_error("Backup message not found for alarm translation. This should never happen.");
      }
      std::string details = primary_details.value_or(backup_details.value());
      std::string description = primary_description.value_or(backup_description.value());
      bool in_locale = primary_description.has_value() && primary_details.has_value();
      std::optional<time_point> final_reset_time = std::nullopt;
      if (reset_time.has_value()) {
        final_reset_time = timepoint_from_milliseconds(reset_time.value());
      }
      activations.emplace_back(alarm_id, activation_id, description, details,
                               static_cast<tfc::snitch::api::active_e>(activation_level),
                               static_cast<tfc::snitch::level_e>(alarm_level), alarm_latching,
                               timepoint_from_milliseconds(activation_time), final_reset_time, in_locale);
    };
    for (auto& activation : activations) {
      // TODO: This is not great would be better to do this in a large query
      // As of now we are doing a query for each activation to get the variables
      std::vector<std::pair<std::string, std::string>> variables;
      db_ << fmt::format("SELECT variable_key, variable_value FROM AlarmVariables WHERE activation_id = {};",
                         activation.activation_id) >>
          [&](std::string key, std::string value) { variables.emplace_back(key, value); };
      fmt::dynamic_format_arg_store<fmt::format_context> store;
      for (auto& [key, value] : variables) {
        store.push_back(fmt::arg(key.c_str(), value));
      }
      activation.details = fmt::vformat(activation.details, store);
      activation.description = fmt::vformat(activation.description, store);
    }
    return activations;
  }

  // Note. Use `echo -n "value" | sha1sum` to not hash the newline character and
  // match the results from this function.
  [[nodiscard]] static auto get_sha1(std::string_view value) -> std::string {
    // Calculate sha1
    std::array<unsigned char, SHA_DIGEST_LENGTH> sha1_res;
    SHA1(reinterpret_cast<const unsigned char*>(value.data()), value.size(), sha1_res.data());

    // Convert sha1 to ascii
    std::string sha1_ascii;
    sha1_ascii.reserve(SHA_DIGEST_LENGTH * 2);
    for (std::size_t i = 0; i < SHA_DIGEST_LENGTH; i++) {
      sha1_ascii += fmt::format("{:02x}", sha1_res[i]);
    }
    return sha1_ascii;
  }

  static constexpr tfc::snitch::api::time_point timepoint_from_milliseconds(std::int64_t ms) {
    return tfc::snitch::api::time_point(std::chrono::milliseconds(ms));
  }

private:
  static std::int64_t milliseconds_since_epoch(std::optional<tfc::snitch::api::time_point> tp) {
    tfc::snitch::api::time_point value =
        tp.value_or(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()));
    return milliseconds_since_epoch_base(value);
  }

  static constexpr std::int64_t milliseconds_since_epoch_base(tfc::snitch::api::time_point tp) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
  }

  // TODO: Want this to work
  // static_assert(milliseconds_since_epoch_base(timepoint_from_milliseconds(1000)) == 1000);

  error_log log_{};
  sqlite::database db_;
};
}  // namespace tfc::themis
