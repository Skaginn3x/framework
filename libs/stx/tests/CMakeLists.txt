find_package(ut CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(mp-units CONFIG REQUIRED)

add_executable(test_glaze_meta test_glaze_meta.cpp)

target_link_libraries(test_glaze_meta
  PRIVATE
    tfc::stx
    Boost::ut
    glaze::glaze
    fmt::fmt
    mp-units::systems
    mp-units::core
)

add_test(
  NAME
    test_glaze_meta
  COMMAND
    test_glaze_meta
)

add_executable(test_asio_condition_variable test_asio_condition_variable.cpp)
target_link_libraries(test_asio_condition_variable
  PRIVATE
    tfc::stx
    fmt::fmt
    Boost::ut
)

add_test(
  NAME
    test_asio_condition_variable
  COMMAND
    test_asio_condition_variable
)
