#include <alarm_database.hpp>
#include <boost/ut.hpp>
#include <iostream>
#include <tfc/progbase.hpp>
#include <tfc/snitch/common.hpp>

namespace ut = boost::ut;

using boost::ut::operator""_test;
using boost::ut::operator|;
using boost::ut::operator/;
using boost::ut::expect;
using boost::ut::throws;
using tfc::themis::alarm_database;

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  "sha1sum calculations correctness"_test =
      [](auto& test_data) {
        std::string input = test_data.first;
        std::string expected = test_data.second;
        expect(alarm_database::get_sha1(input) == expected)
            << fmt::format("{} != {}", alarm_database::get_sha1(input), expected);
      } |
      std::vector<std::pair<std::string, std::string>>{ { "msgshort msg", "654e9cca8eb076653201325a5cf07ed24ee71b52" },
                                                        { "this is a test", "fa26be19de6bff93f70bc2308434e4a440bbad02" },
                                                        { "another test", "afc8edc74ae9e7b8d290f945a6d613f1d264a2b2" } };

  "Database correctness check"_test = [] {
    tfc::themis::alarm_database alarm_db(true);

    // Inserted alarm matches fetched alarm
    expect(alarm_db.list_alarms().size() == 0);
    auto insert_id = alarm_db.register_alarm_en("tfc_id", "msg", "short msg", false, tfc::snitch::level_e::info);
    auto alarms = alarm_db.list_alarms();
    expect(alarms.size() == 1);
    expect(alarms.at(0).tfc_id == "tfc_id");
    expect(alarms.at(0).translations.size() == 1);  // English is always present
    expect(alarms.at(0).sha1sum == alarm_database::get_sha1("msgshort msg"));
    expect(alarms.at(0).lvl == tfc::snitch::level_e::info);
    expect(alarms.at(0).alarm_id == insert_id);
    auto iterator = alarms.at(0).translations.find("en");
    expect(iterator != alarms.at(0).translations.end());
    expect(iterator->second.description == "msg") << iterator->second.description;
    expect(iterator->second.details == "short msg") << iterator->second.details;

    // Translations are attached to alarms
    alarm_db.add_alarm_translation(alarm_db.list_alarms().at(0).alarm_id, "es", "some spanish maybe",
                                   "this is is also spanish believe me please");
    alarms = alarm_db.list_alarms();
    expect(alarms.size() == 1);
    expect(alarms.at(0).translations.size() == 2);  // english and spanish
    iterator = alarms.at(0).translations.find("es");
    expect(iterator != alarms.at(0).translations.end());
    expect(iterator->second.description == "some spanish maybe");
    expect(iterator->second.details == "this is is also spanish believe me please");

    alarm_db.add_alarm_translation(alarm_db.list_alarms().at(0).alarm_id, "is", "Some icelandic really",
                                   "This is really some icelandic");
    alarms = alarm_db.list_alarms();
    expect(alarms.size() == 1);
    expect(alarms.at(0).translations.size() == 3);  // english, spanish and icelandic
    iterator = alarms.at(0).translations.find("is");
    expect(iterator != alarms.at(0).translations.end());
    expect(iterator->second.description == "Some icelandic really");
    expect(iterator->second.details == "This is really some icelandic");

    // Verify there is no sql logic error
    auto activations = alarm_db.list_activations(
        "es", 0, 10, tfc::snitch::level_e::unknown, tfc::snitch::api::state_e::all,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 0);

    activations = alarm_db.list_activations(
        "es", 0, 10, tfc::snitch::level_e::unknown, tfc::snitch::api::state_e::active,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 0);

    activations = alarm_db.list_activations(
        "es", 0, 10, tfc::snitch::level_e::info, tfc::snitch::api::state_e::active,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 0);

    activations = alarm_db.list_activations(
        "es", 0, 10, tfc::snitch::level_e::info, tfc::snitch::api::state_e::all,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 0);

    // Add some activations
    for (int i = 0; i < 10; i++) {
      auto activation_id = alarm_db.set_alarm(insert_id, {});
      expect(alarm_db.reset_alarm(activation_id));
    }
    // Insert an invalid activation
    expect(throws([&] { [[maybe_unused]] auto _ = alarm_db.set_alarm(999, {}); }));

    // Verify our inserts
    activations = alarm_db.list_activations(
        "es", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::state_e::all,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 10) << activations.size();
    activations = alarm_db.list_activations(
        "es", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::state_e::active,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 0) << activations.size();
    activations = alarm_db.list_activations(
        "es", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::state_e::inactive,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 10) << activations.size();
  };
  "Fallback to en if locale is not available"_test = [] {
    tfc::themis::alarm_database alarm_db(true);
    auto insert_id = alarm_db.register_alarm_en("tfc_id", "description", "details", false, tfc::snitch::level_e::info);
    alarm_db.add_alarm_translation(alarm_db.list_alarms().at(0).alarm_id, "es", "spanish description", "spanish details");
    [[maybe_unused]] auto _ = alarm_db.set_alarm(insert_id, {});
    auto activations = alarm_db.list_activations(
        "is", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::state_e::active,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 1);
    expect(activations.at(0).description == "description") << activations.at(0).description;
    expect(activations.at(0).details == "details") << activations.at(0).details;
    expect(activations.at(0).in_requested_locale == false);

    activations = alarm_db.list_activations(
        "es", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::state_e::active,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 1);
    expect(activations.at(0).description == "spanish description") << activations.at(0).description;
    expect(activations.at(0).details == "spanish details") << activations.at(0).details;
    expect(activations.at(0).in_requested_locale == true);
  };
  "String formatting with variables"_test = [] {
    tfc::themis::alarm_database alarm_db(true);
    // Add a variable alarm
    auto var_alarm_id = alarm_db.register_alarm_en("tfc_id", "{var}", "{var}", false, tfc::snitch::level_e::info);
    expect(alarm_db.list_alarms().size() == 1);
    [[maybe_unused]] auto _ = alarm_db.set_alarm(var_alarm_id, { { "var", "10.0" } });
    auto activations = alarm_db.list_activations(
        "en", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::state_e::active,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 1);
    expect(activations.at(0).alarm_id == var_alarm_id);
    expect(activations.at(0).description == "10.0");
    expect(activations.at(0).details == "10.0");

    var_alarm_id = alarm_db.register_alarm_en("tfc_id", "description {var} {var2} {var3}", "details {var} {var2} {var3}",
                                              false, tfc::snitch::level_e::warning);
    expect(alarm_db.list_alarms().size() == 2);
    alarm_db.add_alarm_translation(var_alarm_id, "es", "spanish description {var} {var2} {var3}",
                                   "spanish details {var} {var2} {var3}");
    _ = alarm_db.set_alarm(var_alarm_id, { { "var", "10.0" }, { "var2", "20.0" }, { "var3", "30.0" } });
    activations = alarm_db.list_activations(
        "en", 0, 10000, tfc::snitch::level_e::warning, tfc::snitch::api::state_e::active,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 1);
    expect(activations.at(0).alarm_id == var_alarm_id);
    expect(activations.at(0).description == "description 10.0 20.0 30.0") << activations.at(0).description;
    expect(activations.at(0).details == "details 10.0 20.0 30.0") << activations.at(0).details;
    activations = alarm_db.list_activations(
        "es", 0, 10000, tfc::snitch::level_e::warning, tfc::snitch::api::state_e::active,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 1);
    expect(activations.at(0).alarm_id == var_alarm_id);
    expect(activations.at(0).description == "spanish description 10.0 20.0 30.0") << activations.at(0).description;
    expect(activations.at(0).details == "spanish details 10.0 20.0 30.0") << activations.at(0).details;
    activations = alarm_db.list_activations(
        "is", 0, 10000, tfc::snitch::level_e::warning, tfc::snitch::api::state_e::active,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 1);
    expect(activations.at(0).alarm_id == var_alarm_id);
    expect(activations.at(0).description == "description 10.0 20.0 30.0") << activations.at(0).description;
    expect(activations.at(0).details == "details 10.0 20.0 30.0") << activations.at(0).details;
  };
  "Test the millisecond time storage"_test = [] {
    auto min_time = alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::min());
    auto max_time = alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max());
    auto time = alarm_database::timepoint_from_milliseconds(0);
    expect(time.time_since_epoch().count() == 0);
    time = alarm_database::timepoint_from_milliseconds(1000);
    expect(time.time_since_epoch().count() == 1000);
    time = alarm_database::timepoint_from_milliseconds(1000000);
    expect(time.time_since_epoch().count() == 1000000);
    time = alarm_database::timepoint_from_milliseconds(1000000000);
    expect(time.time_since_epoch().count() == 1000000000);
    time = alarm_database::timepoint_from_milliseconds(1000000000000);
    expect(time.time_since_epoch().count() == 1000000000000);
    time = alarm_database::timepoint_from_milliseconds(1000000000000000);
    expect(time.time_since_epoch().count() == 1000000000000000);

    tfc::themis::alarm_database db(true);
    auto alarm_id = db.register_alarm_en("tfc_id", "description", "details", false, tfc::snitch::level_e::info);
    time = alarm_database::timepoint_from_milliseconds(1);
    auto activation_id = db.set_alarm(alarm_id, {}, time);
    auto activations =
        db.list_activations("en", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::state_e::all, min_time, max_time);
    expect(activations.size() == 1);
    expect(activations.at(0).set_timestamp == time);
    expect(!activations.at(0).reset_timestamp.has_value());
    auto reset_time = alarm_database::timepoint_from_milliseconds(2);
    expect(db.reset_alarm(activation_id, reset_time));
    activations =
        db.list_activations("en", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::state_e::all, min_time, max_time);
    expect(!db.is_alarm_active(alarm_id));
    expect(activations.size() == 1);
    expect(activations.at(0).set_timestamp == time);
    expect(activations.at(0).reset_timestamp.has_value());
    expect(activations.at(0).reset_timestamp == reset_time);
  };
  "You should not be able to set an already set alarm"_test = [] {
    tfc::themis::alarm_database db(true);
    auto alarm_id = db.register_alarm_en("tfc_id", "description", "details", false, tfc::snitch::level_e::info);
    auto activation_id = db.set_alarm(alarm_id, {});
    expect(throws([&] { [[maybe_unused]] auto _ = db.set_alarm(alarm_id, {}); }));
    expect(db.reset_alarm(activation_id));
    expect(!db.reset_alarm(activation_id));
  };
  "Verify alarm id when replacing reinstantiating alarms"_test = [] {
    tfc::themis::alarm_database db(true);
    auto alarm_id = db.register_alarm_en("tfc_id", "description", "details", false, tfc::snitch::level_e::info);
    auto new_alarm_id = db.register_alarm_en("tfc_id", "description", "details", false, tfc::snitch::level_e::info);
    auto not_the_same = db.register_alarm_en("tfc_id2", "description", "details", false, tfc::snitch::level_e::info);
    expect(alarm_id == new_alarm_id);
    expect(alarm_id != not_the_same);
  };
  "re-registering the same alarm should update registered_at"_test = [] {
    tfc::themis::alarm_database db(true);
    auto tp = tfc::themis::alarm_database::timepoint_from_milliseconds(0);
    [[maybe_unused]] auto alarm_id =
        db.register_alarm_en("tfc_id", "description", "details", false, tfc::snitch::level_e::info, tp);
    auto alarms = db.list_alarms();
    expect(alarms.size() == 1);
    expect(alarms.at(0).registered_at == tp);

    tp = tfc::themis::alarm_database::timepoint_from_milliseconds(1);
    alarm_id = db.register_alarm_en("tfc_id", "description", "details", false, tfc::snitch::level_e::info, tp);
    alarms = db.list_alarms();
    expect(alarms.size() == 1);
    expect(alarms.at(0).registered_at == tp);
  };
  "registering an alarm should result in a clean-slate"_test = [] {
    auto db = tfc::themis::alarm_database(true);
    auto alarm_id = db.register_alarm_en("tfc_id", "description", "details", false, tfc::snitch::level_e::info);
    [[maybe_unused]] auto activation_id = db.set_alarm(alarm_id, {});
    expect(db.is_alarm_active(alarm_id));
    auto new_alarm_id = db.register_alarm_en("tfc_id", "description", "details", false, tfc::snitch::level_e::info);
    expect(alarm_id == new_alarm_id);
    expect(!db.is_alarm_active(alarm_id));
  };
  "empty alarm list"_test = [] {
    auto db = tfc::themis::alarm_database(true);
    auto alarms = db.list_alarms();
    expect(alarms.size() == 0);
  };
  "Second time an alarm is registered it should get the correct insert id"_test = [] {
    auto db = tfc::themis::alarm_database(true);
    auto alarm_id = db.register_alarm_en("tfc_id", "description", "details", false, tfc::snitch::level_e::info);
    auto alarm_id2 = db.register_alarm_en("tfc_id", "description", "details", false, tfc::snitch::level_e::info);
    auto alarms = db.list_alarms();
    expect(alarm_id == alarm_id2);
    expect(alarms.size() == 1);
    expect(alarms.at(0).alarm_id == alarm_id);
  };

  "Second time an alarm is registered it should get the correct insert id with a different alarm registered in the middle"_test =
      [] {
        auto db = tfc::themis::alarm_database(true);
        auto alarm_id_first_time =
            db.register_alarm_en("tfc_id", "description", "details", false, tfc::snitch::level_e::info);
        auto alarms = db.list_alarms();
        expect(alarms.size() == 1);
        auto other_alarm = db.register_alarm_en("tfc_other", "description", "details", false, tfc::snitch::level_e::info);
        auto alarm_id_second_time =
            db.register_alarm_en("tfc_id", "description", "details", false, tfc::snitch::level_e::info);
        alarms = db.list_alarms();
        expect(alarm_id_first_time == alarm_id_second_time);
        expect(alarm_id_first_time != other_alarm);
        expect(alarms.size() == 2);
        expect(alarms.at(0).alarm_id == alarm_id_first_time);
      };
}
