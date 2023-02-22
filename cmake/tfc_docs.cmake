# Used for storing all directories where public headers are.
set(TFC_FRAMEWORK_PUBLIC_HEADER_DIRS "" CACHE INTERNAL "")
macro(add_library_to_docs target_library)
  # Find public include directories and add to list
  get_target_property(TEMPORARY_HEADER_DIR ${target_library} INTERFACE_INCLUDE_DIRECTORIES)
  list(APPEND TFC_FRAMEWORK_PUBLIC_HEADER_DIRS ${TEMPORARY_HEADER_DIR})
  set(TFC_FRAMEWORK_PUBLIC_HEADER_DIRS ${TFC_FRAMEWORK_PUBLIC_HEADER_DIRS} CACHE INTERNAL "")
endmacro()

# Used for storing all directories where public headers are.
set(TFC_FRAMEWORK_EXAMPLES "" CACHE INTERNAL "")
macro(add_example_to_docs target_example)
  get_target_property(TEMPORARY_EXAMPLES ${target_example} SOURCES)
  message("CPP FILES ${CMAKE_CURRENT_SOURCE_DIR}${TEMPORARY_EXAMPLES}")
  list(APPEND TFC_FRAMEWORK_EXAMPLES ${CMAKE_CURRENT_SOURCE_DIR}/${TEMPORARY_EXAMPLES})
  set(TFC_FRAMEWORK_EXAMPLES ${TFC_FRAMEWORK_EXAMPLES} CACHE INTERNAL "")
endmacro()