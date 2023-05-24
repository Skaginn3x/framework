project(stx)
cmake_minimum_required(VERSION 3.21)

add_library(stx INTERFACE)
add_library(tfc::stx ALIAS stx)

target_include_directories(stx
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:inc/public>
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(tfc_PACKAGE_NAME tfc-stx)
set(tfc_INSTALL_CMAKEDIR ${CMAKE_INSTALL_DATADIR}/${tfc_PACKAGE_NAME})

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/${tfc_PACKAGE_NAME}ConfigVersion.cmake"
  VERSION 0.0.1
  COMPATIBILITY SameMajorVersion
)

install(TARGETS stx
  EXPORT ${tfc_PACKAGE_NAME}Targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT Development
  BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/tfcConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/${tfc_PACKAGE_NAME}Config.cmake"
  INSTALL_DESTINATION ${tfc_INSTALL_CMAKEDIR}
)

install(EXPORT ${tfc_PACKAGE_NAME}Targets DESTINATION ${tfc_INSTALL_CMAKEDIR})
install(
  FILES
    "${PROJECT_BINARY_DIR}/${tfc_PACKAGE_NAME}ConfigVersion.cmake"
    "${PROJECT_BINARY_DIR}/${tfc_PACKAGE_NAME}Config.cmake"
  DESTINATION ${tfc_INSTALL_CMAKEDIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/inc/public/ DESTINATION include)

install(
  EXPORT ${tfc_PACKAGE_NAME}Targets
  NAMESPACE tfc::
  DESTINATION ${tfc_INSTALL_CMAKEDIR}
  COMPONENT Development
)
