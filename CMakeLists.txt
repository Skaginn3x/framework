cmake_minimum_required(VERSION 3.25)
project(tfc-framework)

enable_testing()

# Add the cmake folder so the FindSphinx module is found
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" "${CMAKE_CURRENT_LIST_DIR}/cmake/findModules")

include(tfc_config)
include(tfc_options)
include(tfc_warnings)
include(tfc_docs)

add_subdirectory(exes)
add_subdirectory(libs)
add_subdirectory(docs)
