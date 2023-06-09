# Used for storing all directories where public headers are.
set(TFC_FRAMEWORK_PUBLIC_HEADER_DIRS "" CACHE INTERNAL "")
macro(add_library_to_docs target_library)
  # Find public include directories and add to list
  get_target_property(TEMPORARY_SOURCE_DIR ${target_library} SOURCE_DIR)
  list(APPEND TFC_FRAMEWORK_PUBLIC_HEADER_DIRS ${TEMPORARY_SOURCE_DIR}/inc)
  set(TFC_FRAMEWORK_PUBLIC_HEADER_DIRS ${TFC_FRAMEWORK_PUBLIC_HEADER_DIRS} CACHE INTERNAL "")
endmacro()

# Used for storing all example.cpp files for doxygen to find them
set(TFC_FRAMEWORK_EXAMPLES "" CACHE INTERNAL "")
macro(add_example_to_docs target_example)
  get_target_property(TEMPORARY_EXAMPLES ${target_example} SOURCES)
  list(APPEND TFC_FRAMEWORK_EXAMPLES ${CMAKE_CURRENT_SOURCE_DIR}/${TEMPORARY_EXAMPLES})
  set(TFC_FRAMEWORK_EXAMPLES ${TFC_FRAMEWORK_EXAMPLES} CACHE INTERNAL "")
endmacro()

function(tfc_add_example_no_test EXAMPLE_TARGET cpp_file)
  add_executable(${EXAMPLE_TARGET} ${cpp_file})
  # Tell doxygen where to find this example
  add_example_to_docs(${EXAMPLE_TARGET})
endfunction()

function(tfc_add_example EXAMPLE_TARGET cpp_file)
  tfc_add_example_no_test(${EXAMPLE_TARGET} ${cpp_file})
  add_test(
    NAME
    ${EXAMPLE_TARGET}
    COMMAND
    ${EXAMPLE_TARGET}
  )
endfunction()
