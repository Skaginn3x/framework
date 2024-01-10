add_executable(ipc-ruler src/ipc-ruler.cpp)

find_package(Boost REQUIRED COMPONENTS program_options)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDBUSPLUS REQUIRED IMPORTED_TARGET GLOBAL sdbusplus)

find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_path(SQLITE_MODERN_CPP_INCLUDE_DIRS "sqlite_modern_cpp.h")
target_include_directories(ipc-ruler PRIVATE $<BUILD_INTERFACE:${SQLITE_MODERN_CPP_INCLUDE_DIRS}>)

target_link_libraries(ipc-ruler
  PUBLIC
    tfc::ipc
    tfc::base
    tfc::confman
    tfc::logger
    tfc::dbus_util
    Boost::program_options
    PkgConfig::SDBUSPLUS
    unofficial::sqlite3::sqlite3
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

tfc_systemd_service_file(ipc-ruler "TFC Inter-process communication ruler")
