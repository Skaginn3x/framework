add_subdirectory(configure_options)
add_subdirectory(dbus_util)
add_subdirectory(stx)
add_subdirectory(logger)
add_subdirectory(progbase)
add_subdirectory(confman)
add_subdirectory(ipc)
add_subdirectory(rpc)
add_subdirectory(ec)
add_subdirectory(operation_mode)

add_library(tfc INTERFACE)
add_library(tfc::tfc ALIAS tfc)
target_link_libraries(tfc
  INTERFACE
    tfc::stx
    tfc::base
    tfc::logger
    tfc::confman
    tfc::ipc
)

# Let's go ahead and install everything properly
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(tfc_PACKAGE_NAME tfc)
set(tfc_INSTALL_CMAKEDIR ${CMAKE_INSTALL_DATADIR}/${tfc_PACKAGE_NAME})

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${tfc_PACKAGE_NAME}ConfigVersion.cmake"
  VERSION ${CMAKE_PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/${tfc_PACKAGE_NAME}Config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${tfc_PACKAGE_NAME}Config.cmake"
  INSTALL_DESTINATION ${tfc_INSTALL_CMAKEDIR}
)

install(EXPORT ${tfc_PACKAGE_NAME}Targets DESTINATION ${tfc_INSTALL_CMAKEDIR})
install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${tfc_PACKAGE_NAME}ConfigVersion.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${tfc_PACKAGE_NAME}Config.cmake"
  DESTINATION ${tfc_INSTALL_CMAKEDIR}
)

install(
  EXPORT ${tfc_PACKAGE_NAME}Targets
  NAMESPACE tfc::
  DESTINATION ${tfc_INSTALL_CMAKEDIR}
)
