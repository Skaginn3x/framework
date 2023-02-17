cmake_minimum_required(VERSION 3.25)
project(tfc-framework)

include(cmake/config.cmake)
include(cmake/options.cmake)
include(cmake/warnings.cmake)

add_subdirectory(exes)
add_subdirectory(libs)
