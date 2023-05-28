
add_executable(confman_mocks_example main.cpp)

find_package(ut CONFIG REQUIRED)
target_link_libraries(confman_mocks_example
  PUBLIC
    Boost::ut
    tfc::mock_confman
)
