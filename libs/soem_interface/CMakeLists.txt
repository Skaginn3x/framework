project(soem_interface)
add_library(soem_interface INTERFACE)
add_library(tfc::soem_interface ALIAS soem_interface)

target_include_directories(soem_interface
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
)

find_package(soem CONFIG REQUIRED)

target_link_libraries(soem_interface
  INTERFACE
    soem
)

include(tfc_install)
tfc_install_lib(soem_interface)
