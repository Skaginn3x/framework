# npm i
# react-scripts build


add_custom_target(
   cockpit-IPC-build
   npm install --no-package-lock
   COMMAND GENERATE_SOURCEMAP=false BUILD_PATH='./build' ${CMAKE_CURRENT_SOURCE_DIR}/node_modules/react-scripts/bin/react-scripts.js build
   COMMAND rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/node_modules
   COMMAND rm -rf ${CMAKE_CURRENT_BINARY_DIR}/build
   COMMAND mv ${CMAKE_CURRENT_SOURCE_DIR}/build ${CMAKE_CURRENT_BINARY_DIR}
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)


include(GNUInstallDirs)
install(
  DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/build
  DESTINATION
    /usr/share/cockpit/ipc
  CONFIGURATIONS Release
)
install(
  FILES
   manifest.json
  DESTINATION
    /usr/share/cockpit/ipc/
  CONFIGURATIONS Release
)