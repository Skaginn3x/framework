add_executable(confman_mocks_example main.cpp)

find_package(Boost REQUIRED COMPONENTS )
find_package(ut CONFIG REQUIRED)

target_link_libraries(confman_mocks_example
  PUBLIC
    Boost::ut
    tfc::base
    tfc::mock_confman
    tfc::logger
    Boost::boost
)
