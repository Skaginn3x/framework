project(logger)
cmake_minimum_required(VERSION 3.21)

add_library(logger
  src/logger.cpp
)
add_library(tfc::logger ALIAS logger)
target_include_directories(logger
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:inc/public>
  PRIVATE
    inc
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

add_subdirectory(tests)
