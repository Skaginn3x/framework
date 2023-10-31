project(motor)
cmake_minimum_required(VERSION 3.28)

add_library(motor)
add_library(tfc::motor ALIAS motor)

target_sources(motor
  PUBLIC
    FILE_SET CXX_MODULES FILES
    src/motor.cxx
    src/impl.cxx
    src/motors/mock.cxx
    src/motors/ethercat.cxx
)

find_package(magic_enum CONFIG REQUIRED)
find_package(mp-units CONFIG REQUIRED)

target_link_libraries(motor
  PUBLIC
    tfc::base
    tfc::logger
    tfc::stx
    tfc::dbus_util
    tfc::confman
    magic_enum::magic_enum
    mp-units::mp-units
)

add_library_to_docs(motor)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

#include(tfc_install)
#tfc_install_lib(motor)
