add_executable(ipc-ruler src/ipc-ruler.cpp)


find_package(PkgConfig REQUIRED)
pkg_check_modules(SDBUSPLUS REQUIRED IMPORTED_TARGET GLOBAL sdbusplus)

target_link_libraries(ipc-ruler
  PUBLIC
    tfc::ipc
    tfc::base
    tfc::confman
    tfc::logger
    tfc::dbus_util
    PkgConfig::SDBUSPLUS
)

include(GNUInstallDirs)

install(
  TARGETS
    ipc-ruler
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}
  CONFIGURATIONS Release
)

install(
  TARGETS
    ipc-ruler
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}/debug/
  CONFIGURATIONS Debug
)

include(tfc_systemd)
tfc_systemd_service_file(ipc-ruler "TFC Inter-process communication ruler")
