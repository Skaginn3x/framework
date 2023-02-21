cmake_minimum_required(VERSION 3.25)
project(tfc-framework)

enable_testing()

# Add the cmake folder so the FindSphinx module is found
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/findModules" ${CMAKE_MODULE_PATH})

include(cmake/config.cmake)
include(cmake/options.cmake)
include(cmake/warnings.cmake)
include(cmake/docs.cmake)

add_subdirectory(exes)
add_subdirectory(libs)
add_subdirectory(docs)
