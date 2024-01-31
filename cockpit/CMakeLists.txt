
configure_file(${CMAKE_CURRENT_LIST_DIR}/src/variables.js.in ${CMAKE_CURRENT_LIST_DIR}/src/variables.js @ONLY)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/build
  COMMAND npm install --no-package-lock
  COMMAND GENERATE_SOURCEMAP=false BUILD_PATH='./build' ${CMAKE_CURRENT_SOURCE_DIR}/node_modules/react-scripts/bin/react-scripts.js build
  COMMAND rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/node_modules
  COMMAND rm -rf ${CMAKE_CURRENT_BINARY_DIR}/build
  COMMAND mv ${CMAKE_CURRENT_SOURCE_DIR}/build ${CMAKE_CURRENT_BINARY_DIR}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(cockpit-build ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/build)

include(GNUInstallDirs)

install(
  DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/build/
  DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/cockpit/ipc
  CONFIGURATIONS Release
)
install(
  FILES
    manifest.json
  DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/cockpit/ipc/
  CONFIGURATIONS Release
)