add_library(sml src/dbus_interface.cpp)
add_library(tfc::sml ALIAS sml)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDBUSPLUS REQUIRED IMPORTED_TARGET GLOBAL sdbusplus)
find_package(fmt CONFIG REQUIRED)
find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")

target_link_libraries(sml
  PUBLIC
    PkgConfig::SDBUSPLUS
    tfc::dbus_util
    tfc::stx
    tfc::configure_options
    tfc::logger
    fmt::fmt
)

target_include_directories(sml
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${BEXT_SML_INCLUDE_DIRS}>
)

add_library_to_docs(tfc::sml)

include(tfc_install)
tfc_install_lib(sml)
