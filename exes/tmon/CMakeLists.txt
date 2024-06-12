project(tmon)

add_executable(tmon
  src/main.cpp
  src/app_operation_mode.cpp
  src/state_machine_owner.cpp
)

find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_path(SQLITE_MODERN_CPP_INCLUDE_DIRS "sqlite_modern_cpp.h")
target_include_directories(tmon
  PUBLIC
    inc
    ${BEXT_SML_INCLUDE_DIRS}
  PRIVATE
    $<BUILD_INTERFACE:${SQLITE_MODERN_CPP_INCLUDE_DIRS}>
)

find_package(Boost REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDBUSPLUS REQUIRED IMPORTED_TARGET GLOBAL sdbusplus)

target_link_libraries(tmon
  PUBLIC
    tfc::base
    tfc::logger
    tfc::stx
    tfc::ipc
    tfc::operation_mode
    tfc::confman
    Boost::boost
    PkgConfig::SDBUSPLUS
    unofficial::sqlite3::sqlite3
)

include(tfc_split_debug_info)
tfc_split_debug_info(operations)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

include(GNUInstallDirs)
install(
  TARGETS
    tmon
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}
  CONFIGURATIONS Release
)

install(
  TARGETS
    tmon
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}/debug/
  CONFIGURATIONS Debug
)
include(tfc_systemd)
tfc_systemd_service_file(tmon "TFC Daemon")
