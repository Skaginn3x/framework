project(configure_options)
cmake_minimum_required(VERSION 3.21)


configure_file(${CMAKE_CURRENT_LIST_DIR}/configure_options.hpp.in ${CMAKE_CURRENT_LIST_DIR}/inc/public/tfc/configure_options.hpp)

add_library(configure_options INTERFACE)
add_library(tfc::configure_options ALIAS configure_options)

target_include_directories(configure_options
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
)

configure_file(${CMAKE_CURRENT_LIST_DIR}/dbus-tfc.conf.in ${CMAKE_CURRENT_LIST_DIR}/generated/dbus-tfc.conf)

include(tfc_install)
tfc_install_lib(configure_options)


## Place dbus config in /etc
set(CMAKE_INSTALL_PREFIX, "/")
include(GNUInstallDirs)
install(
  FILES
    generated/dbus-tfc.conf
  DESTINATION
    ${CMAKE_INSTALL_SYSCONFDIR}/dbus-1/system.d/tfc.conf
  CONFIGURATIONS Release
)
