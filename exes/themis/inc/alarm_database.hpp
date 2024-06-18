#pragma once

#include <filesystem>
#include <string>
#include <tfc/progbase.hpp>
#include <sqlite_modern_cpp.h>
#include <fmt/format.h>
#include <fmt/printf.h>

namespace tfc::themis {
inline auto config_file_name_populate_dir() -> std::string {
  auto const file{ base::make_config_file_name(base::get_exe_name(), "db") };
  std::filesystem::create_directories(file.parent_path());
  return file.string();
}
class alarm_database {
public:
  explicit alarm_database(bool in_memory = false) : db_(in_memory ? ":memory:" : config_file_name_populate_dir()) {
    db_ << R"(
CREATE TABLE IF NOT EXISTS Alarms(
  alarm_id INTEGER PRIMARY KEY,
  tfc_id TEXT NOT NULL,
  sha1sum TEXT NOT NULL,
  alarm_level INTEGER NOT NULL,
  short_msg_en TEXT NOT NULL,
  msg_en TEXT NOT NULL,
  alarm_latching BOOLEAN NOT NULL,
  UNIQUE(tfc_id, sha1sum) ON CONFLICT IGNORE
);
             )";
    db_ << R"(
CREATE TABLE IF NOT EXISTS AlarmTranslation(
  sha1sum TEXT PRIMARY KEY,
  locale TEXT NOT NULL,
  short_msg TEXT NOT NULL,
  msg TEXT NOT NULL,
  FOREIGN KEY(sha1sum) REFERENCES Alarms(sha1sum)
);
)";
    // Timestamp stored as long integer to get millisecond precision
    // And because sqlite does not offer a native timestamp type.
    db_ << R"(
CREATE TABLE IF NOT EXISTS AlarmActivations(
  activation_id INTEGER PRIMARY KEY,
  alarm_id INTEGER NOT NULL,
  activation_time LONG INTEGER NOT NULL,
  activation_level INTEGER NOT NULL,
  FOREIGN KEY(alarm_id) REFERENCES Alarms(alarm_id)
);
)";
    db_ << R"(
CREATE TABLE IF NOT EXISTS AlarmAcks(
  activation_id INTEGER,
  ack_timestamp LONG INTEGER NOT NULL,
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
  [[nodiscard]] auto register_alarm(std::string_view tfc_id, std::string_view msg, std::string_view short_msg, bool latching, std::uint32_t alarm_level) -> std::int64_t {
    db_ << fmt::format("INSERT INTO Alarms(tfc_id, sha1sum, alarm_level, short_msg_en, msg_en, alarm_latching) VALUES('{}','{}',{},'{}','{}',{});", tfc_id, msg, alarm_level, short_msg, msg, latching ? 1 : 0);
    return db_.last_insert_rowid();
  }
private:
  sqlite::database db_;
};
}
