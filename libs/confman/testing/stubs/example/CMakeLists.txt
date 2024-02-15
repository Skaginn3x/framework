add_executable(confman_stubs_example main.cpp)

find_package(Boost REQUIRED COMPONENTS )

target_link_libraries(confman_stubs_example
  PUBLIC
    tfc::base
    tfc::stub_confman
    tfc::logger
    Boost::boost
)
