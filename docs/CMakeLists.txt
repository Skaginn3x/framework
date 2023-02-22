find_package(Doxygen REQUIRED)
find_package(Sphinx REQUIRED)

# List all header files so that build fails if some are missing
# List of all public headers in the project,
# This is useful because if doxygen depends on these files
# It will be rebuilt when they are modified
set(TFC_FRAMEWORK_PUBLIC_HEADERS "")
foreach (HEADER_DIRECTORY ${TFC_FRAMEWORK_PUBLIC_HEADER_DIRS})
  file(GLOB_RECURSE HEADERS_LOCAL ${HEADER_DIRECTORY}/*.hpp)
  list(APPEND TFC_FRAMEWORK_PUBLIC_HEADERS ${HEADERS_LOCAL})
  set(TFC_FRAMEWORK_PUBLIC_HEADERS ${TFC_FRAMEWORK_PUBLIC_HEADERS} PARENT_SCOPE)
endforeach ()
# Also monitor the examples
foreach (EXAMPLE ${TFC_FRAMEWORK_EXAMPLES})
  list(APPEND TFC_FRAMEWORK_PUBLIC_HEADERS ${EXAMPLE})
  set(TFC_FRAMEWORK_PUBLIC_HEADERS ${TFC_FRAMEWORK_PUBLIC_HEADERS} PARENT_SCOPE)
endforeach ()

# Join all the public header directories in a format that doxygen understands
list(JOIN TFC_FRAMEWORK_PUBLIC_HEADER_DIRS " " TFC_FRAMEWORK_PUBLIC_HEADER_DIRS_FORMATTED)
list(JOIN TFC_FRAMEWORK_EXAMPLES " " TFC_FRAMEWORK_EXAMPLES_FORMATTED)

set(DOXYGEN_INPUT_DIR ${TFC_FRAMEWORK_PUBLIC_HEADER_DIRS_FORMATTED})
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
set(DOXYGEN_ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/doxygen_assets)
set(DOXYGEN_EXTRA_STYLE_SHEET "${DOXYGEN_ASSETS_DIR}/css/doxygen-awesome.css ${DOXYGEN_ASSETS_DIR}/css/doxygen-awesome-sidebar-only.css")
set(DOXYGEN_LOGO ${DOXYGEN_ASSETS_DIR}/images/s3x.svg)
message("FORMATTED EXAMPLES ${TFC_FRAMEWORK_EXAMPLES_FORMATTED}")
set(DOXYGEN_EXAMPLES ${TFC_FRAMEWORK_EXAMPLES_FORMATTED})

#Replace variables inside @@ with the current values
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR}) #Doxygen won't create this for us
add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
  DEPENDS ${TFC_FRAMEWORK_PUBLIC_HEADERS} # Currently not populated
  COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
  MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
  COMMENT "Generating docs")

add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})

set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sphinx)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

# Only regenerate Sphinx when:
# - Doxygen has rerun
# - Our doc files have been updated
# - The Sphinx config has been updated
add_custom_command(OUTPUT ${SPHINX_INDEX_FILE}
  COMMAND
  ${SPHINX_EXECUTABLE} -b html
  # Tell Breathe where to find the Doxygen output
  -Dbreathe_projects.tfc=${DOXYGEN_OUTPUT_DIR}/xml
  ${SPHINX_SOURCE} ${SPHINX_BUILD}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
  # Other docs files you want to track should go here (or in some variable)
  ${CMAKE_CURRENT_SOURCE_DIR}/index.rst
  ${DOXYGEN_INDEX_FILE}
  MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
  COMMENT "Generating documentation with Sphinx")

# Nice named target so we can run the job easily
add_custom_target(Sphinx ALL DEPENDS ${SPHINX_INDEX_FILE})

# Add an install target to install the docs
include(GNUInstallDirs)
install(DIRECTORY ${SPHINX_BUILD}
  DESTINATION ${CMAKE_INSTALL_DOCDIR})