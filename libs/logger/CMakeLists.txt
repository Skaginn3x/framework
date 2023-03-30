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

add_library(systemdlib INTERFACE)
find_package(PkgConfig)
pkg_check_modules(SYSTEMD "libsystemd")
if(${SYSTEMD_FOUND})
  message("Found systemd")
else()
  error("Systemd not found")
endif()
target_include_directories(systemdlib INTERFACE ${SYSTED_INCLUDE_DIRS})
target_link_libraries(systemdlib INTERFACE ${SYSTEMD_LIBRARIES})

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(logger
  PUBLIC
    fmt::fmt
  PRIVATE
    spdlog::spdlog
    tfc::base
    systemdlib
)

add_library_to_docs(tfc::logger)

add_subdirectory(tests)
