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

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/mylibConfigVersion.cmake"
  VERSION 0.0.1
  COMPATIBILITY SameMajorVersion
)

install(TARGETS stx
  EXPORT mylibTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT Development
  BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/mylibConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/mylibConfig.cmake"
    INSTALL_DESTINATION lib/cmake/mylib
)

install(EXPORT mylibTargets DESTINATION lib/cmake/mylib)
install(FILES "${PROJECT_BINARY_DIR}/mylibConfigVersion.cmake"
    "${PROJECT_BINARY_DIR}/mylibConfig.cmake"
    DESTINATION lib/cmake/mylib)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/inc/public/ DESTINATION include)

install(
  EXPORT mylibTargets
  NAMESPACE tfc::
  DESTINATION lib/cmake/mylib
  COMPONENT Development
)
