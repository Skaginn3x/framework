
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

message("\n\n\n\n")
message("\n\n\n\n")
message("---------------------------------------------------------")
message("sphinx build: ${SPHINX_BUILD}")
# Set the SPHINX_BUILD variable to the Sphinx build directory
# set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sphinx)

# Use execute_process to run the ls command and list the contents of SPHINX_BUILD
execute_process(COMMAND ls ${CMAKE_CURRENT_BINARY_DIR}/build
        RESULT_VARIABLE LS_RESULT
        OUTPUT_VARIABLE LS_OUTPUT)

# Log the output of the ls command
message(STATUS "Listing the contents of ${CMAKE_CURRENT_BINARY_DIR}/build")
message(STATUS "${LS_OUTPUT}")
message("---------------------------------------------------------")

message("\n\n\n\n")
message("\n\n\n\n")

install(
  DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/build/
  DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/cockpit/tfc
  CONFIGURATIONS Release
)
install(
  FILES
    manifest.json
  DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/cockpit/tfc/
  CONFIGURATIONS Release
)