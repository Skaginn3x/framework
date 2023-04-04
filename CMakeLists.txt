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

if (BUILD_DOCS)
  add_subdirectory(docs)
endif()

# Clang format all files

file(GLOB_RECURSE ALL_SOURCE_FILES libs/**/*.cpp libs/**/*.hpp exes/**/*.cpp exes/**/*.hpp)
add_custom_target(
  clangformat-fix
  COMMAND clang-format
  --style=file
  -i
  ${ALL_SOURCE_FILES}
)
add_custom_target(
  clangformat-dry
  COMMAND clang-format
  --style=file
  -i
  --dry-run
  ${ALL_SOURCE_FILES}
)


# from https://stackoverflow.com/questions/18968979/how-to-make-colorized-message-with-cmake
string(ASCII 27 Esc)
set(ColourReset "${Esc}[m")
set(Red         "${Esc}[31m")
set(Green       "${Esc}[32m")
message("${Green}Using ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}${ColourReset}")