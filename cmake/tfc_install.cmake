
set(THIS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

function(tfc_install TARGET)
  message(WARNING "Current directory: ${THIS_DIRECTORY}")

  include(GNUInstallDirs)
  include(CMakePackageConfigHelpers)

  set(tfc_PACKAGE_NAME tfc-${TARGET})
  set(tfc_INSTALL_CMAKEDIR ${CMAKE_INSTALL_DATADIR}/${tfc_PACKAGE_NAME})

  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${tfc_PACKAGE_NAME}ConfigVersion.cmake"
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )

  install(TARGETS ${TARGET}
    EXPORT ${tfc_PACKAGE_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT Development
    BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
  )

  configure_package_config_file(
    "${THIS_DIRECTORY}/tfcConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${tfc_PACKAGE_NAME}Config.cmake"
    INSTALL_DESTINATION ${tfc_INSTALL_CMAKEDIR}
  )

  install(EXPORT ${tfc_PACKAGE_NAME}Targets DESTINATION ${tfc_INSTALL_CMAKEDIR})
  install(
    FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${tfc_PACKAGE_NAME}ConfigVersion.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${tfc_PACKAGE_NAME}Config.cmake"
    DESTINATION ${tfc_INSTALL_CMAKEDIR})
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/inc/public/ DESTINATION include)

  install(
    EXPORT ${tfc_PACKAGE_NAME}Targets
    NAMESPACE tfc::
    DESTINATION ${tfc_INSTALL_CMAKEDIR}
    COMPONENT Development
  )
endfunction()
