project(ipc)
cmake_minimum_required(VERSION 3.21)

add_library(ipc
  STATIC
    src/ipc.cpp
)
add_library(tfc::ipc ALIAS ipc)

find_path(AZMQ_INCLUDE_DIRS "azmq/actor.hpp")

target_include_directories(ipc
  PUBLIC
    inc/public
    ${AZMQ_INCLUDE_DIRS}
  PRIVATE
    inc
)

find_package(Boost REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(fmt CONFIG REQUIRED)

target_link_libraries(ipc
  PUBLIC
    tfc::stx
    tfc::logger
    tfc::base
    Boost::boost
    fmt::fmt

  PRIVATE
    libzmq-static
)

add_library_to_docs(tfc::ipc)

add_subdirectory(tests)