project(themis)
add_executable(themis src/main.cpp)

find_package(Boost REQUIRED COMPONENTS program_options)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDBUSPLUS REQUIRED IMPORTED_TARGET GLOBAL sdbusplus)

find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(OpenSSL CONFIG REQUIRED)

find_path(SQLITE_MODERN_CPP_INCLUDE_DIRS "sqlite_modern_cpp.h")
target_include_directories(themis
    PRIVATE
      $<BUILD_INTERFACE:${SQLITE_MODERN_CPP_INCLUDE_DIRS}>
    PUBLIC
      inc
)

target_link_libraries(themis
    PUBLIC
    tfc::ipc
    tfc::base
    tfc::confman
    tfc::logger
    tfc::dbus_util
    tfc::snitch
    Boost::program_options
    PkgConfig::SDBUSPLUS
    unofficial::sqlite3::sqlite3
    OpenSSL::Crypto
)

add_subdirectory(tests)

include(tfc_split_debug_info)
tfc_split_debug_info(themis)

include(GNUInstallDirs)

install(
    TARGETS
    themis
    DESTINATION
    ${CMAKE_INSTALL_BINDIR}
    CONFIGURATIONS Release
)

install(
    TARGETS
    themis
    DESTINATION
    ${CMAKE_INSTALL_BINDIR}/debug/
    CONFIGURATIONS Debug
)

include(tfc_systemd)
tfc_systemd_service_file(themis "TFC themis - alarm system")
