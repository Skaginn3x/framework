project(snitch)
cmake_minimum_required(VERSION 3.21)

add_library(snitch
  src/snitch.cpp
)
add_library(tfc::snitch ALIAS snitch)

target_include_directories(snitch
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
)

find_package(Boost REQUIRED)
find_package(unofficial-sdbusplus CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(snitch
  PUBLIC
    tfc::base
    tfc::logger
    tfc::stx
    tfc::dbus_util
    magic_enum::magic_enum
  PRIVATE
    unofficial::sdbusplus
    Boost::boost
)

add_library_to_docs(tfc::snitch)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

include(tfc_install)
tfc_install_lib(snitch)
