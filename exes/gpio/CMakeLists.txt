add_executable(gpio src/main.cpp src/gpio.cpp)

find_path(AZMQ_INCLUDE_DIRS "azmq/actor.hpp")
find_path(GPIOD_INCLUDE_DIRS "gpiod.hpp")

target_include_directories(gpio
  PUBLIC
    inc
    ${AZMQ_INCLUDE_DIRS}
    ${GPIOD_INCLUDE_DIRS}
)

target_link_directories(gpio
  PUBLIC
    $<$<CONFIG:Debug>:${GPIOD_INCLUDE_DIRS}/../debug/lib>
    $<$<CONFIG:Release>:${GPIOD_INCLUDE_DIRS}/../lib>
)

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(glaze CONFIG REQUIRED)

target_link_libraries(gpio
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
    gpio
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}
  CONFIGURATIONS Release
)

install(
  TARGETS
    gpio
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}/debug/
  CONFIGURATIONS Debug
)
