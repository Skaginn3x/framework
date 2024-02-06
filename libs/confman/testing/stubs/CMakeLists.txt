
add_library(stub_confman INTERFACE)
add_library(tfc::stub_confman ALIAS stub_confman)

target_include_directories(stub_confman
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(stub_confman
  INTERFACE
    tfc::confman
)

include(tfc_install)
tfc_install_lib(stub_confman)

add_library_to_docs(tfc::stub_confman)

if(BUILD_EXAMPLES)
  add_subdirectory(example)
endif()
