project(motor)
cmake_minimum_required(VERSION 3.21)

add_library(motor
  src/motor.cpp
)
add_library(tfc::motor ALIAS motor)

target_include_directories(motor
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
)

find_package(magic_enum CONFIG REQUIRED)
find_package(mp-units CONFIG REQUIRED)

target_link_libraries(motor
  PUBLIC
    tfc::base
    tfc::logger
    tfc::stx
    tfc::dbus_util
    magic_enum::magic_enum
    mp-units::mp-units
)

add_library_to_docs(tfc::motor)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

include(tfc_install)
tfc_install_lib(motor)
