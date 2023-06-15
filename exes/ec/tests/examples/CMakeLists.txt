tfc_add_example_no_test(ec_run ec_run.cpp)

find_package(Boost REQUIRED COMPONENTS program_options)
target_link_libraries(ec_run tfc::ec Boost::program_options)