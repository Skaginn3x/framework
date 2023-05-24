project(dbus_util)
cmake_minimum_required(VERSION 3.21)

add_library(dbus_util src/dbus_util.cpp src/compile_tests.cpp)
add_library(tfc::dbus_util ALIAS dbus_util)
target_include_directories(dbus_util
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:inc/public>
)

find_package(unofficial-sdbusplus CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)

target_link_libraries(dbus_util
  PUBLIC
    unofficial::sdbusplus
    magic_enum::magic_enum
    tfc::stx
    tfc::configure_options
)

add_library_to_docs(tfc::dbus_util)

add_subdirectory(tests)

include(tfc_install)
tfc_install_lib(dbus_util)
