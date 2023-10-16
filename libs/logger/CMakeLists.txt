project(logger)
cmake_minimum_required(VERSION 3.28)

add_library(logger)
target_sources(logger
  PUBLIC
  FILE_SET cxx_modules TYPE CXX_MODULES FILES
  inc/public/tfc/logger.cxx
)
add_library(tfc::logger ALIAS logger)
target_include_directories(logger
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>
)

find_package(Boost REQUIRED COMPONENTS )

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(logger
  PUBLIC
    fmt::fmt
    Boost::boost
  PRIVATE
    spdlog::spdlog
    tfc::base
    tfc::stx
)

add_library_to_docs(tfc::logger)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

#include(tfc_install)
#tfc_install_lib(logger)
