add_executable(signal_source src/main.cpp)

find_path(AZMQ_INCLUDE_DIRS "azmq/actor.hpp")

target_include_directories(signal_source
  PUBLIC
    inc
    ${AZMQ_INCLUDE_DIRS}
)

target_link_directories(signal_source
  PUBLIC
    $<$<CONFIG:Debug>:${GPIOD_INCLUDE_DIRS}/../debug/lib>
    $<$<CONFIG:Release>:${GPIOD_INCLUDE_DIRS}/../lib>
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
    gpiodcxx
    gpiod
)

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
