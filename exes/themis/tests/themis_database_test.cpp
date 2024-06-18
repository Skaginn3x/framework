#include <iostream>
#include <tfc/progbase.hpp>
#include <boost/ut.hpp>
#include <alarm_database.hpp>

namespace ut = boost::ut;

using boost::ut::operator""_test;

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  //"Database correctness check"_test = [] {
    tfc::themis::alarm_database alarm_db(false);
    auto insert_id = alarm_db.register_alarm("tfc_id", "msg", "short msg", false, 0);
    std::cout << insert_id << std::endl;
  //};
}
