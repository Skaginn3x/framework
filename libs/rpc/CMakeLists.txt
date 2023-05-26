project(rpc)
cmake_minimum_required(VERSION 3.21)

find_package(glaze CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Boost REQUIRED)
find_package(ZeroMQ REQUIRED)
find_path(AZMQ_INCLUDE_DIRS "azmq/actor.hpp")


add_library(librpc INTERFACE)
add_library(tfc::rpc ALIAS librpc)

target_include_directories(librpc
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
    ${AZMQ_INCLUDE_DIRS}
)

target_link_libraries(librpc
  INTERFACE
    tfc::base
    tfc::logger
    glaze::glaze
    fmt::fmt
    libzmq-static
    Boost::boost
)
add_library_to_docs(tfc::rpc)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()
