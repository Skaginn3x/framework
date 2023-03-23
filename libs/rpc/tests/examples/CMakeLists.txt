find_package(glaze CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

tfc_add_example(json_rpc_example json_rpc_example.cpp)
target_link_libraries(json_rpc_example glaze::glaze Boost::program_options tfc::base tfc::logger tfc::rpc tfc::stx)