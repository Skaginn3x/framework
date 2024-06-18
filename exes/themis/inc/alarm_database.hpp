#pragma once

#include <fmt/format.h>
#include <openssl/sha.h>
#include <sqlite_modern_cpp.h>
#include <array>
#include <filesystem>
#include <string>
#include <tfc/progbase.hpp>
#include <tfc/snitch/common.hpp>

namespace tfc::themis {

using enum tfc::snitch::level_e;
using enum tfc::snitch::api::active_e;

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
  sha1sum TEXT,
  locale TEXT NOT NULL,
  short_msg TEXT NOT NULL,
  msg TEXT NOT NULL,
  PRIMARY KEY(sha1sum, locale) ON CONFLICT REPLACE,
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
  [[nodiscard]] auto register_alarm(std::string_view tfc_id,
                                    std::string_view msg,
                                    std::string_view short_msg,
                                    bool latching,
                                    tfc::snitch::level_e alarm_level) -> std::uint64_t {
    std::string sha1_ascii = get_sha1(fmt::format("{}{}", msg, short_msg));
    db_ << fmt::format(
        "INSERT INTO Alarms(tfc_id, sha1sum, alarm_level, short_msg_en, msg_en, alarm_latching) "
        "VALUES('{}','{}',{},'{}','{}',{});",
        tfc_id, sha1_ascii, static_cast<std::uint8_t>(alarm_level), short_msg, msg, latching ? 1 : 0);
    std::int64_t last_insert_rowid = db_.last_insert_rowid();
    if (last_insert_rowid < 0) {
      throw std::runtime_error("Failed to insert alarm into database");
    }
    return static_cast<std::uint64_t>(last_insert_rowid);
  }

  [[nodiscard]] auto list_alarms() -> std::vector<tfc::snitch::api::alarm> {
    std::unordered_map<std::uint64_t, tfc::snitch::api::alarm> alarms;
    std::string query = R"(
SELECT
  alarm_id,
  tfc_id,
  Alarms.sha1sum,
  alarm_level,
  Alarms.short_msg_en,
  Alarms.msg_en,
  alarm_latching,
  locale,
  AlarmTranslation.short_msg,
  AlarmTranslation.msg
FROM Alarms
LEFT JOIN AlarmTranslation
ON Alarms.sha1sum = AlarmTranslation.sha1sum;
)";
    // Accept the second table paramters as unique ptr's as the values can be null, and we want to preserve that
    db_ << query >> [&](std::uint64_t alarm_id, std::string tfc_id, std::string sha1sum, std::uint8_t alarm_level,
                        std::string short_msg_en, std::string msg_en, bool alarm_latching,
                        std::unique_ptr<std::string> locale, std::unique_ptr<std::string> translated_short_msg,
                        std::unique_ptr<std::string> translated_msg) {
      auto iterator = alarms.find(alarm_id);
      if (iterator == alarms.end()) {
        alarms.insert({ alarm_id, tfc::snitch::api::alarm{ alarm_id,
                                                           tfc_id,
                                                           short_msg_en,
                                                           msg_en,
                                                           sha1sum,
                                                           static_cast<tfc::snitch::level_e>(alarm_level),
                                                           alarm_latching,
                                                           {} } });
      }
      if (locale != nullptr && translated_short_msg != nullptr && translated_msg != nullptr) {
        alarms[alarm_id].translations.insert({ *locale, { *translated_short_msg, *translated_msg } });
      }
    };
    std::vector<tfc::snitch::api::alarm> alarm_list;
    for (auto& [_, alarm] : alarms) {
      alarm_list.push_back(alarm);
    }
    return alarm_list;
  }

  auto add_alarm_translation(std::string_view sha1sum,
                             std::string_view locale,
                             std::string_view short_msg,
                             std::string_view msg) -> void {
    db_ << fmt::format("INSERT INTO AlarmTranslation(sha1sum, locale, short_msg, msg) VALUES('{}','{}','{}','{}');", sha1sum,
                       locale, short_msg, msg);
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
    for (size_t i = 0; i < SHA_DIGEST_LENGTH; i++) {
      sha1_ascii += fmt::format("{:02x}", sha1_res[i]);
    }
    return sha1_ascii;
  }

private:
  sqlite::database db_;
};
}  // namespace tfc::themis
