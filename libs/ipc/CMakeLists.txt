project(ipc)
cmake_minimum_required(VERSION 3.21)

add_library(ipc
  src/ipc.cpp
  src/dbus_server_iface_mock.cpp # todo make ipc_test target
)
add_library(tfc::ipc ALIAS ipc)

find_path(AZMQ_INCLUDE_DIRS "azmq/actor.hpp")

target_include_directories(ipc
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${AZMQ_INCLUDE_DIRS}>
)

find_package(Boost REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)

target_link_libraries(ipc
  PUBLIC
    tfc::stx
    tfc::logger
    tfc::base
    tfc::dbus_util
    tfc::confman
    Boost::boost
    fmt::fmt
    glaze::glaze
    $<IF:$<BOOL:${ZeroMQ_LIBRARY}>,${ZeroMQ_LIBRARY},${ZeroMQ_STATIC_LIBRARY}>
)

add_library_to_docs(tfc::ipc)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

include(tfc_install)
tfc_install_lib(ipc)
