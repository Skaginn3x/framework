add_executable(signal_source src/main.cpp)

find_path(AZMQ_INCLUDE_DIRS "azmq/actor.hpp")

target_include_directories(signal_source
  PUBLIC
    ${AZMQ_INCLUDE_DIRS}
)

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(glaze CONFIG REQUIRED)

target_link_libraries(signal_source
  PUBLIC
    tfc::ipc
    tfc::base
    tfc::logger
    tfc::confman
    Boost::program_options
    glaze::glaze
)

include(tfc_split_debug_info)
tfc_split_debug_info(signal_source)

include(GNUInstallDirs)
install(
  TARGETS
    signal_source
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}
  CONFIGURATIONS Release
)

install(
  TARGETS
    signal_source
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}/debug/
  CONFIGURATIONS Debug
)
