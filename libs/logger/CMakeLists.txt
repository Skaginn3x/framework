project(logger)
cmake_minimum_required(VERSION 3.21)

add_library(logger STATIC
  src/logger.cpp
)
add_library(tfc::logger ALIAS logger)
target_include_directories(logger
  PUBLIC
    inc/public
  PRIVATE
    inc
)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(logger
  PRIVATE
    fmt::fmt
    spdlog::spdlog
)
