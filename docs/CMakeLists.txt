project(tfc-docs)
find_package(Doxygen REQUIRED)

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
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/sphinx/doxygen)
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
set_target_properties(Doxygen PROPERTIES
  EXCLUDE_FROM_ALL ON
)


# Let's create the read the docs html

# Create a virtual environment for python using this method:
set(VENV_PATH "${CMAKE_CURRENT_BINARY_DIR}/.venv")
# https://discourse.cmake.org/t/possible-to-create-a-python-virtual-env-from-cmake-and-then-find-it-with-findpython3/1132
find_package (Python3 REQUIRED COMPONENTS Interpreter)
execute_process (COMMAND "${Python3_EXECUTABLE}" -m venv "${VENV_PATH}")

# Here is the trick
## update the environment with VIRTUAL_ENV variable (mimic the activate script)
set (ENV{VIRTUAL_ENV} "${VENV_PATH}")
## change the context of the search
set (Python3_FIND_VIRTUALENV FIRST)
## unset Python3_EXECUTABLE because it is also an input variable (see documentation, Artifacts Specification section)
unset (Python3_EXECUTABLE)
## Launch a new search
find_package (Python3 COMPONENTS Interpreter Development)

add_custom_command(
  OUTPUT venv_setup
  COMMAND ${VENV_PATH}/bin/pip install -r requirements.txt
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sphinx)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)
set(SPHINX_EXECUTABLE "${VENV_PATH}/bin/sphinx-build")

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
  DEPENDS venv_setup
  # Other docs files you want to track should go here (or in some variable)
  ${CMAKE_CURRENT_SOURCE_DIR}/index.rst
  ${CMAKE_CURRENT_SOURCE_DIR}/design/cm.md
  ${DOXYGEN_INDEX_FILE}
  MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
  COMMENT "Generating documentation with Sphinx")

# Nice named target so we can run the job easily
add_custom_target(ReadTheDocs ALL DEPENDS ${SPHINX_INDEX_FILE})
set_target_properties(ReadTheDocs PROPERTIES
  EXCLUDE_FROM_ALL ON
)

# Now let's create the slideshow

set(REVEALJS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/promo-slides)
set(REVEALJS_BUILD ${CMAKE_CURRENT_BINARY_DIR}/promo-slides)
set(REVEALJS_INDEX_FILE ${SPHINX_BUILD}/index.html)
add_custom_command(OUTPUT ${REVEALJS_INDEX_FILE}
    COMMAND
    ${SPHINX_EXECUTABLE} -b html
    ${REVEALJS_SOURCE} ${REVEALJS_BUILD}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS venv_setup
    # Other docs files you want to track should go here (or in some variable)
    ${REVEALJS_SOURCE}/index.rst
    MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
    COMMENT "Generating slideshow with Sphinx and Revealjs")

# Nice named target so we can run the job easily
add_custom_target(Revealjs ALL DEPENDS ${REVEALJS_INDEX_FILE})
set_target_properties(Revealjs PROPERTIES
    EXCLUDE_FROM_ALL ON
)

# Add an install target to install the docs
include(GNUInstallDirs)
install(
  DIRECTORY
    ${SPHINX_BUILD}/
  DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/cockpit/${PROJECT_NAME}
  CONFIGURATIONS Release
)
install(
  FILES
    manifest.json
  DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/cockpit/${PROJECT_NAME}/
  CONFIGURATIONS Release
)
