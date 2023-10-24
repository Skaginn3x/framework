find_package(ut CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)
find_package(fmt CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
add_executable(observer_test observer_test.cpp)

target_link_libraries(observer_test PRIVATE tfc::confman Boost::ut glaze::glaze)

add_test(
  NAME
    observer_test
  COMMAND
    observer_test
)

add_executable(read_only_test read_only_test.cpp)

target_link_libraries(read_only_test PRIVATE tfc::confman Boost::ut glaze::glaze)

add_test(
    NAME
    read_only_test
    COMMAND
    read_only_test
)

add_executable(confman_test confman_test.cpp)

target_link_libraries(confman_test
  PRIVATE
    tfc::confman
    tfc::ipc
    Boost::ut
    glaze::glaze
)

add_test(
  NAME
    confman_test
  COMMAND
    confman_test
)

add_executable(confman_unit_test confman_unit_test.cpp)

target_link_libraries(confman_unit_test
  PRIVATE
    tfc::confman
    tfc::mock_confman
    Boost::ut
    glaze::glaze
)

add_test(
  NAME
    confman_unit_test
  COMMAND
    confman_unit_test
)

add_executable(file_storage_test ../../src/file_storage.cpp file_storage_test.cpp)

target_link_libraries(file_storage_test
  PRIVATE
    tfc::confman
    Boost::ut
    fmt::fmt
    glaze::glaze
)

add_test(
  NAME
    file_storage_test
  COMMAND
    file_storage_test
)

target_include_directories(file_storage_test
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)
