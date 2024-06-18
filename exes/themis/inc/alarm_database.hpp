#pragma once

#include <fmt/format.h>
#include <openssl/sha.h>
#include <sqlite_modern_cpp.h>
#include <sqlite_modern_cpp/log.h>
#include <array>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <tfc/logger.hpp>
#include <tfc/progbase.hpp>
#include <tfc/snitch/common.hpp>

namespace tfc::themis {

using enum tfc::snitch::level_e;
using tfc::snitch::api::time_point;

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
  UNIQUE(tfc_id, sha1sum) ON CONFLICT IGNORE
);
             )";
    db_ << R"(
CREATE TABLE IF NOT EXISTS AlarmTranslations(
  sha1sum TEXT NOT NULL,
  alarm_id INTEGER NOT NULL,
  locale TEXT NOT NULL,
  short_msg TEXT NOT NULL,
  msg TEXT NOT NULL,
  PRIMARY KEY(sha1sum, locale) ON CONFLICT REPLACE
  FOREIGN KEY(alarm_id) REFERENCES Alarms(alarm_id)
);
)";
    // Timestamp stored as long integer to get millisecond precision
    // And because sqlite does not offer a native timestamp type.
    db_ << R"(
CREATE TABLE IF NOT EXISTS AlarmActivations(
  activation_id LONG INTEGER PRIMARY KEY,
  alarm_id INTEGER NOT NULL,
  activation_time LONG INTEGER NOT NULL,
  activation_level BOOLEAN NOT NULL,
  FOREIGN KEY(alarm_id) REFERENCES Alarms(alarm_id)
);
)";
    db_ << R"(
CREATE TABLE IF NOT EXISTS AlarmAcks(
  activation_id INTEGER,
  ack_timestamp LONG INTEGER NOT NULL,
  UNIQUE(activation_id) ON CONFLICT IGNORE,
  FOREIGN KEY(activation_id) REFERENCES AlarmActivations(activation_id)
);
)";
    db_ << R"(
CREATE TABLE IF NOT EXISTS AlarmVariables(
  activation_id INTEGER,
  variable_key TEXT NOT NULL,
  variable_value TEXT NOT NULL,
  FOREIGN KEY(activation_id) REFERENCES AlarmActivations(activation_id)
);
)";
  }
  /**
   * @brief Register an alarm in the database
   * @param tfc_id The TFC ID of the alarm
   * @param msg The message of the alarm
   * @param short_msg The short message of the alarm
   * @param latching Whether the alarm is latching
   * @param alarm_level The alarm level
   * @return The alarm ID
   */
  [[nodiscard]] auto register_alarm_en(std::string_view tfc_id,
                                       std::string_view msg,
                                       std::string_view short_msg,
                                       bool latching,
                                       tfc::snitch::level_e alarm_level) -> std::uint64_t {
    std::string sha1_ascii = get_sha1(fmt::format("{}{}", msg, short_msg));
    std::uint64_t alarm_id = 0;
    try {
      db_ << "BEGIN;";
      db_ << fmt::format(
          "INSERT INTO Alarms(tfc_id, sha1sum, alarm_level, alarm_latching) "
          "VALUES('{}','{}',{},{});",
          tfc_id, sha1_ascii, static_cast<std::uint8_t>(alarm_level), latching ? 1 : 0);
      auto insert_id = db_.last_insert_rowid();
      if (insert_id < 0) {
        throw std::runtime_error("Failed to insert alarm into database");
      }
      alarm_id = static_cast<std::uint64_t>(insert_id);
      add_alarm_translation(sha1_ascii, alarm_id, "en", short_msg, msg);
      db_ << "COMMIT;";
    } catch (std::exception& e) {
      // Rollback the transaction and rethrow
      db_ << "ROLLBACK;";
      throw e;
    }
    return alarm_id;
  }

  [[nodiscard]] auto list_alarms() -> std::vector<tfc::snitch::api::alarm> {
    std::unordered_map<std::uint64_t, tfc::snitch::api::alarm> alarms;
    std::string query = R"(
SELECT
  Alarms.alarm_id,
  Alarms.tfc_id,
  Alarms.sha1sum,
  alarm_level,
  Alarms.alarm_latching,
  AlarmTranslations.locale,
  AlarmTranslations.short_msg,
  AlarmTranslations.msg
FROM Alarms
LEFT JOIN AlarmTranslations
ON Alarms.sha1sum = AlarmTranslations.sha1sum;
)";
    // Accept the second table paramters as unique ptr's as the values can be null, and we want to preserve that
    db_ << query >> [&](std::uint64_t alarm_id, std::string tfc_id, std::string sha1sum, std::uint8_t alarm_level,
                        bool alarm_latching, std::optional<std::string> locale,
                        std::optional<std::string> translated_short_msg, std::optional<std::string> translated_msg) {
      auto iterator = alarms.find(alarm_id);
      if (iterator == alarms.end()) {
        alarms.insert(
            { alarm_id,
              tfc::snitch::api::alarm{
                  alarm_id, tfc_id, sha1sum, static_cast<tfc::snitch::level_e>(alarm_level), alarm_latching, {} } });
      }
      if (locale.has_value() && translated_short_msg.has_value() && translated_msg.has_value()) {
        alarms[alarm_id].translations.insert({ locale.value(), { translated_short_msg.value(), translated_msg.value() } });
      }
    };
    std::vector<tfc::snitch::api::alarm> alarm_list;
    for (auto& [_, alarm] : alarms) {
      alarm_list.push_back(alarm);
    }
    return alarm_list;
  }

  auto add_alarm_translation(std::string_view sha1sum,
                             std::uint64_t alarm_id,
                             std::string_view locale,
                             std::string_view short_msg,
                             std::string_view msg) -> void {
    auto query = fmt::format(
        "INSERT INTO AlarmTranslations(sha1sum, alarm_id, locale, short_msg, msg) VALUES('{}', {}, '{}','{}','{}');",
        sha1sum, alarm_id, locale, short_msg, msg);
    std::cerr << query << std::endl;
    db_ << query;
  }

  auto set_alarm(std::uint64_t alarm_id,
                 std::unordered_map<std::string_view, std::string_view> variables,
                 std::optional<tfc::snitch::api::time_point> tp = {}) -> void {
    db_ << fmt::format("INSERT INTO AlarmActivations(alarm_id, activation_time, activation_level) VALUES({},{},1)", alarm_id,
                       milliseconds_since_epoch(tp));
    std::int64_t last_insert_rowid = db_.last_insert_rowid();
    if (last_insert_rowid < 0) {
      throw std::runtime_error("Failed to insert activation into database");
    }

    for (auto& [key, value] : variables) {
      db_ << fmt::format("INSERT INTO AlarmVariables(activation_id, variable_key, variable_value) VALUES({},{},'{}');",
                         last_insert_rowid, key, value);
    }
  }
  auto reset_alarm(std::uint64_t alarm_id, std::optional<tfc::snitch::api::time_point> tp = {}) -> void {
    db_ << fmt::format("INSERT INTO AlarmActivations(alarm_id, activation_time, activation_level) VALUES({},{},0)", alarm_id,
                       milliseconds_since_epoch(tp));
  }

  auto ack_alarm(std::uint64_t activation_id, std::optional<tfc::snitch::api::time_point> tp = {}) -> void {
    db_ << fmt::format("INSERT INTO AlarmAcks(activation_id, ack_timestamp) VALUES({},{});", activation_id,
                       milliseconds_since_epoch(tp));
  }

  auto ack_all_alarms(std::optional<tfc::snitch::api::time_point> tp = {}) -> void {
    // TODO: This is not 100% but we should be able to do this in a single query. Very fast.
    //  Gonna build up some more elaborate test cases and dig into this when that is complete
    db_ << fmt::format(
        "INSERT INTO AlarmAcks(activation_id, ack_timestamp) SELECT DISTINCT activation_id, {} FROM AlarmActivations;",
        milliseconds_since_epoch(tp));
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
  activation_level,
  AlarmTranslations.short_msg,
  AlarmTranslations.msg,
  AlarmTranslations.locale,
  Alarms.alarm_latching,
  Alarms.alarm_level
FROM AlarmActivations
JOIN Alarms on (Alarms.alarm_id = AlarmActivations.alarm_id)
LEFT OUTER JOIN AlarmTranslations on (Alarms.sha1sum = AlarmTranslations.sha1sum)
WHERE (AlarmTranslations.locale = '{}' OR AlarmTranslations.locale = 'en') AND activation_time >= {} AND activation_time <= {})",
                                              locale, milliseconds_since_epoch(start), milliseconds_since_epoch(end));
    if (level != tfc::snitch::level_e::unknown) {
      populated_query += fmt::format(" AND alarm_level = {}", static_cast<std::uint8_t>(level));
    }
    if (active != tfc::snitch::api::active_e::all) {
      populated_query += fmt::format(" AND activation_level = {}", static_cast<std::uint8_t>(active));
    }
    populated_query += fmt::format(" LIMIT {} OFFSET {};", count, start_count);

    std::unordered_map<std::uint64_t, tfc::snitch::api::activation> activations;

    db_ << populated_query >> [&](std::uint64_t activation_id, std::uint64_t alarm_id, std::int64_t activation_time,
                                  bool activation_level, std::optional<std::string> translated_short_msg,
                                  std::optional<std::string> translated_msg, std::optional<std::string> tlocale,
                                  bool alarm_latching, std::uint8_t alarm_level) {
      // This callback can be called multiple times for the same activation_id
      // Once with the locale requested and once with the english locale
      // I am going to try and solve that with a group by clause

      // For now we do this in c++, if we have value and the underlying element is not found or there is no element and the locale is wrong
      if (translated_msg.has_value() && translated_short_msg.has_value() && tlocale.has_value()) {
        auto iterator = activations.find(activation_id);
        if (iterator != activations.end() || iterator->second.locale == locale) {
          activations[activation_id] = { alarm_id,
                                             activation_id,
                                             translated_msg.value(),
                                             translated_short_msg.value(),
                                             tlocale.value(),
                                             activation_level,
                                             static_cast<tfc::snitch::level_e>(alarm_level),
                                             alarm_latching,
                                             timepoint_from_milliseconds(activation_time) };
        }
      }
    };
    std::vector<tfc::snitch::api::activation> ret_value;
    for (auto& [_, activation] : activations) {
      ret_value.push_back(activation);
    }
    return ret_value;
  }

  // Note. Use `echo -n "value" | sha1sum` to not hash the newline character and
  // match the results from this function.
  [[nodiscard]] static auto get_sha1(std::string_view msg) -> std::string {
    // Calculate sha1
    std::array<unsigned char, SHA_DIGEST_LENGTH> sha1_res;
    SHA1(reinterpret_cast<const unsigned char*>(msg.data()), msg.size(), sha1_res.data());

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
