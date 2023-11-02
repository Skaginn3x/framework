project(operations)

add_executable(operations
  src/main.cpp
  src/app_operation_mode.cpp
  src/state_machine_owner.cpp
)

find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")
target_include_directories(operations
  PUBLIC
    inc
    ${BEXT_SML_INCLUDE_DIRS}
)

find_package(Boost REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDBUSPLUS REQUIRED IMPORTED_TARGET GLOBAL sdbusplus)
target_link_libraries(operations
  PUBLIC
    tfc::base
    tfc::logger
    tfc::stx
    tfc::ipc
    tfc::operation_mode
    tfc::confman
    Boost::boost
    PkgConfig::SDBUSPLUS
)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

include(GNUInstallDirs)
install(
  TARGETS
    operations
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}
  CONFIGURATIONS Release
)

install(
  TARGETS
    operations
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}/debug/
  CONFIGURATIONS Debug
)

include(tfc_systemd)
tfc_systemd_service_file(operations "TFC Operation Mode")
