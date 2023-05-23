
add_executable(mocking mocking.cpp)

find_package(ut CONFIG REQUIRED)

target_link_libraries(mocking
  PUBLIC
    tfc::mock
    Boost::ut
)

add_example_to_docs(mocking)
