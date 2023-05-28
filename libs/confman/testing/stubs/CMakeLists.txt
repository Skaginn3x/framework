
add_library(confman_stubs INTERFACE)
add_library(tfc::stubs::confman ALIAS confman_stubs)

target_include_directories(confman_stubs
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(confman_stubs
  INTERFACE
    tfc::confman
)

add_library_to_docs(tfc::stubs::confman)
add_subdirectory(example)
