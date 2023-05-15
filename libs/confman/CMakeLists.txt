project(confman)
cmake_minimum_required(VERSION 3.21)

find_package(glaze CONFIG REQUIRED)
find_path(AZMQ_INCLUDE_DIRS "azmq/actor.hpp")
find_package(fmt CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS )
find_package(unofficial-sdbusplus CONFIG REQUIRED)

add_library(libconfman
  src/confman.cpp
  src/detail/config_dbus_client.cpp
)
add_library(tfc::confman ALIAS libconfman)

target_include_directories(libconfman
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:inc/public>
    ${AZMQ_INCLUDE_DIRS}
  PRIVATE
    inc
)


target_link_libraries(libconfman
  PUBLIC
    Boost::boost
    fmt::fmt
    glaze::glaze
    tfc::base
    tfc::rpc
    tfc::stx
    tfc::dbus_util
  PRIVATE
    unofficial::sdbusplus
)
add_library_to_docs(tfc::confman)

add_subdirectory(tests)
