add_executable(mqtt_bridge
  src/spark_plug_interface.cpp
  src/client.cpp
  src/tfc_to_external.cpp
  src/external_to_tfc.cpp
  src/main.cpp
)

find_package(OpenSSL REQUIRED)

target_include_directories(mqtt_bridge
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/inc
)

target_link_libraries(mqtt_bridge
  PUBLIC
    tfc::base
    tfc::logger
    tfc::stx
    tfc::ipc
    tfc::operation_mode
    tfc::confman
    OpenSSL::SSL OpenSSL::Crypto
    tfc::sparkplug::proto
)

target_compile_definitions(mqtt_bridge
  PUBLIC
    ASYNC_MQTT_USE_TLS
)

include(GNUInstallDirs)
install(
  TARGETS
    mqtt_bridge
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}
  CONFIGURATIONS
    Release
)

install(
  TARGETS
    mqtt_bridge
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}/debug/
  CONFIGURATIONS
    Debug
)

add_subdirectory(tests)

include(tfc_systemd)
tfc_systemd_service_file(mqtt_bridge "Communication bridge between Spark Plug B and TFC")
