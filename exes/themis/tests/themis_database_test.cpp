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
    expect(alarms.at(0).translations.size() == 1); // English is always present
    expect(alarms.at(0).sha1sum == alarm_database::get_sha1("msgshort msg"));
    expect(alarms.at(0).lvl == tfc::snitch::level_e::info);
    expect(alarms.at(0).alarm_id == insert_id);
    auto iterator = alarms.at(0).translations.find("en");
    expect(iterator != alarms.at(0).translations.end());
    expect(iterator->second.description == "msg") << iterator->second.description;
    expect(iterator->second.details == "short msg") << iterator->second.details;

    // Translations are attached to alarms
    alarm_db.add_alarm_translation(alarm_db.list_alarms().at(0).sha1sum, alarm_db.list_alarms().at(0).alarm_id, "es", "some spanish maybe",
                                   "this is is also spanish believe me please");
    alarms = alarm_db.list_alarms();
    expect(alarms.size() == 1);
    expect(alarms.at(0).translations.size() == 2); // english and spanish
    iterator = alarms.at(0).translations.find("es");
    expect(iterator != alarms.at(0).translations.end());
    expect(iterator->second.description == "some spanish maybe");
    expect(iterator->second.details == "this is is also spanish believe me please");

    alarm_db.add_alarm_translation(alarm_db.list_alarms().at(0).sha1sum, alarm_db.list_alarms().at(0).alarm_id, "is", "Some icelandic really",
                                   "This is really some icelandic");
    alarms = alarm_db.list_alarms();
    expect(alarms.size() == 1);
    expect(alarms.at(0).translations.size() == 3); // english, spanish and icelandic
    iterator = alarms.at(0).translations.find("is");
    expect(iterator != alarms.at(0).translations.end());
    expect(iterator->second.description == "Some icelandic really");
    expect(iterator->second.details == "This is really some icelandic");

    // Verify there is no sql logic error
    auto activations = alarm_db.list_activations(
        "es", 0, 10, tfc::snitch::level_e::unknown, tfc::snitch::api::active_e::all,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 0);

    activations = alarm_db.list_activations(
        "es", 0, 10, tfc::snitch::level_e::unknown, tfc::snitch::api::active_e::active,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 0);

    activations = alarm_db.list_activations(
        "es", 0, 10, tfc::snitch::level_e::info, tfc::snitch::api::active_e::active,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 0);

    activations = alarm_db.list_activations(
        "es", 0, 10, tfc::snitch::level_e::info, tfc::snitch::api::active_e::all,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 0);

    // Add some activations
    for(int i = 0; i < 10; i++){
      alarm_db.set_alarm(insert_id, {});
      alarm_db.reset_alarm(insert_id);
    }
    // Insert an invalid activation
    expect(throws([&]{alarm_db.set_alarm(999, {}); }));

    // Verify our inserts
    activations = alarm_db.list_activations(
        "es", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::active_e::all,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 20) << activations.size();
    activations = alarm_db.list_activations(
        "es", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::active_e::active,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 10) << activations.size();
    activations = alarm_db.list_activations(
        "es", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::active_e::inactive,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 10) << activations.size();

  };
  "Fallback to en if locale is not available"_test = []{
    tfc::themis::alarm_database alarm_db(true);
    auto insert_id = alarm_db.register_alarm_en("tfc_id", "description", "details", false, tfc::snitch::level_e::info);
    alarm_db.add_alarm_translation(alarm_db.list_alarms().at(0).sha1sum, alarm_db.list_alarms().at(0).alarm_id, "es", "spanish description", "spanish details");
    alarm_db.set_alarm(insert_id, {});
    auto activations = alarm_db.list_activations(
        "is", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::active_e::active,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 1);
    expect(activations.at(0).description == "description") << activations.at(0).description;
    expect(activations.at(0).details == "details") << activations.at(0).details;

    activations = alarm_db.list_activations(
        "es", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::active_e::active,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 1);
    expect(activations.at(0).description == "spanish description") << activations.at(0).description;
    expect(activations.at(0).details == "spanish details") << activations.at(0).details;
  };
  "String formatting with variables"_test = []{
    tfc::themis::alarm_database alarm_db(true);
    // Add a variable alarm
    auto var_alarm_id = alarm_db.register_alarm_en("tfc_id", "{var}", "{var}", false, tfc::snitch::level_e::info);
    expect(alarm_db.list_alarms().size() == 1);
    alarm_db.set_alarm(var_alarm_id, {{ "var", "10.0"}});
    auto activations = alarm_db.list_activations(
        "en", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::active_e::active,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 1);
    expect(activations.at(0).alarm_id == var_alarm_id);
    expect(activations.at(0).description == "10.0");
    expect(activations.at(0).details == "10.0");

    var_alarm_id = alarm_db.register_alarm_en("tfc_id", "description {var} {var2} {var3}", "details {var} {var2} {var3}", false, tfc::snitch::level_e::warning);
    expect(alarm_db.list_alarms().size() == 2);
    alarm_db.set_alarm(var_alarm_id, {{ "var", "10.0"}, {"var2", "20.0"}, {"var3", "30.0"}});
    activations = alarm_db.list_activations(
        "en", 0, 10000, tfc::snitch::level_e::warning, tfc::snitch::api::active_e::active,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 1);
    expect(activations.at(0).alarm_id == var_alarm_id);
    expect(activations.at(0).description == "description 10.0 20.0 30.0") << activations.at(0).description;
    expect(activations.at(0).details == "details 10.0 20.0 30.0") << activations.at(0).details;
  };
  "ACK the alarm"_test = []{
    //TODO: Do something here.
    tfc::themis::alarm_database alarm_db(true);
    auto insert_id = alarm_db.register_alarm_en("tfc_id", "description", "details", true, tfc::snitch::level_e::info);
    alarm_db.set_alarm(insert_id, {});
    alarm_db.ack_alarm(insert_id);
    auto activations = alarm_db.list_activations(
        "en", 0, 10000, tfc::snitch::level_e::info, tfc::snitch::api::active_e::active,
        tfc::themis::alarm_database::timepoint_from_milliseconds(0),
        tfc::themis::alarm_database::timepoint_from_milliseconds(std::numeric_limits<std::int64_t>::max()));
    expect(activations.size() == 1);
  };
}
