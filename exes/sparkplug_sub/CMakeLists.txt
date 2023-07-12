add_executable(sparkplug_sub src/sparkplug_sub.cpp)

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(async_mqtt_iface CONFIG REQUIRED)

target_link_libraries(sparkplug_sub
  PUBLIC
    tfc::ipc
    tfc::base
    tfc::logger
    tfc::sparkplug::proto
    Boost::program_options
    async_mqtt_iface::async_mqtt_iface
)

include(GNUInstallDirs)
install(
  TARGETS
    sparkplug_sub
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}
  CONFIGURATIONS Release
)

install(
  TARGETS
    sparkplug_sub
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}/debug/
  CONFIGURATIONS Debug
)
