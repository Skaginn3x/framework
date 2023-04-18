add_executable(sd_journal_send sd_journal_send.cpp)
target_link_libraries(sd_journal_send tfc::base tfc::logger)
tfc_add_example(logging_example logging_example.cpp)
target_link_libraries(logging_example tfc::base tfc::logger)