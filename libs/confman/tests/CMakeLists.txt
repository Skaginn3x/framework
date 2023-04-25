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
    tfc::ipc_connector
    Boost::ut
    glaze::glaze
)

add_test(
  NAME
    confman_test
  COMMAND
    confman_test
)

add_executable(dbus_test sdbus_test.cpp)

find_package(unofficial-systemd CONFIG REQUIRED)
find_package(sdbus-c++ CONFIG REQUIRED)
target_link_libraries(dbus_test
  PRIVATE
    unofficial::systemd::libsystemd
    tfc::confman
    tfc::ipc_connector
    Boost::ut
    Boost::program_options
    fmt::fmt
    glaze::glaze
    SDBusCpp::sdbus-c++
)

add_test(
  NAME
    dbus_test
  COMMAND
    dbus_test
)

add_subdirectory(examples)
