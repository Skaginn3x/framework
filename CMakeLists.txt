cmake_minimum_required(VERSION 3.27)
project(tfc-framework
  VERSION
    "2024.6.1"
  DESCRIPTION
    "The tfc framework eases the integration of often used components in different context with mappable IPC and config."
  HOMEPAGE_URL
    https://skaginn3x.github.io/framework/
  LANGUAGES
    CXX
)

set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
include(CTest)

# Add the cmake folder so the FindSphinx module is found
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" "${CMAKE_CURRENT_LIST_DIR}/cmake/findModules")

include(tfc_options)
include(tfc_config)
include(tfc_warnings)
include(tfc_docs)
include(tfc_cpack)

set(PACKAGE_NAME ${PROJECT_NAME})
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(PACKAGE_NAME "${PACKAGE_NAME}-dbg")
endif()
tfc_cpack_init(${PACKAGE_NAME} ${CMAKE_CURRENT_LIST_DIR}/LICENSE ${CMAKE_CURRENT_LIST_DIR}/readme.md)

if (BUILD_EXES)
add_subdirectory(exes)
endif()

add_subdirectory(libs)

if (BUILD_DOCS)
  add_subdirectory(docs)
endif()

if (BUILD_EXAMPLES)
  add_subdirectory(examples)
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
message("${Green}Using ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} for ${CMAKE_SYSTEM_PROCESSOR}${ColourReset}")
message("${Green}Library path: $ENV{LIBRARY_PATH}${ColourReset}")
message("${Green}Build type: ${CMAKE_BUILD_TYPE}${ColourReset}")

feature_summary(
  DESCRIPTION "A summary of TFC options"
  WHAT ALL
)
