cmake_minimum_required(VERSION 3.25)
project(tfc-framework
  VERSION
    0.1.1 # This populates CMAKE_PROJECT_VERSION lower projects will inherit from this one
  DESCRIPTION
    "The tfc framework eases the integration of often used components in different context with mappable IPC and config."
  HOMEPAGE_URL
    https://skaginn3x.github.io/framework/
  LANGUAGES
    CXX
)
enable_testing()


# TODO: Depending on libsystemd should add libcap to the linker as a transitive dep
link_libraries(cap)

# TODO: Depending on dbus-cpp should add libsystemd, Threads to the linker as a transitive dep
find_package(PkgConfig REQUIRED)
pkg_check_modules(Systemd IMPORTED_TARGET GLOBAL libsystemd>=236)
if(NOT TARGET PkgConfig::Systemd)
  message(FATAL_ERROR "libsystemd of version at least 236 is required, but was not found "
      "(if you have systemd in your OS, you may want to install package containing pkgconfig "
      " files for libsystemd library. On Ubuntu, that is libsystemd-dev. "
      " Alternatively, you may turn BUILD_LIBSYSTEMD on for sdbus-c++ to download, build "
      "and incorporate libsystemd as embedded library within sdbus-c++)")
endif()
find_package(Threads REQUIRED)


# Add the cmake folder so the FindSphinx module is found
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" "${CMAKE_CURRENT_LIST_DIR}/cmake/findModules")

include(tfc_options)
include(tfc_config)
include(tfc_warnings)
include(tfc_docs)
include(tfc_install)

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
message("${Green}Using ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} for ${CMAKE_SYSTEM_PROCESSOR}${ColourReset}")