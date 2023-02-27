find_package(glaze CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)
tfc_add_example(json_schema_glaze json_schema_glaze.cpp)
target_link_libraries(json_schema_glaze glaze::glaze Boost::program_options tfc::base tfc::logger)