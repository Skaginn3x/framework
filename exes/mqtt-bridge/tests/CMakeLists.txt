find_package(async_mqtt_iface CONFIG REQUIRED)

add_executable(test_mqtt_bridge
  ../src/spark_plug_interface.cpp
  ../src/signal_names.cpp
  src/test_external_to_tfc.cpp
  src/unit_tests.cpp
)

target_include_directories(test_mqtt_bridge
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(test_mqtt_bridge
  PUBLIC
    tfc::ipc
    Boost::boost
    tfc::sparkplug::proto
    async_mqtt_iface::async_mqtt_iface
)

target_compile_definitions(test_mqtt_bridge
  PUBLIC
    ASYNC_MQTT_USE_TLS
)

add_test(
  NAME
    test_mqtt_bridge
  COMMAND
    test_mqtt_bridge
)

add_executable(mqtt_bridge_integration_tests
  ../src/spark_plug_interface.cpp
  ../src/signal_names.cpp
  src/integration_tests.cpp
)

target_include_directories(mqtt_bridge_integration_tests
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(mqtt_bridge_integration_tests
  PUBLIC
    tfc::ipc
    tfc::sparkplug::proto
    async_mqtt_iface::async_mqtt_iface
)

add_test(
  NAME
    mqtt_bridge_integration_tests
  COMMAND
    mqtt_bridge_integration_tests
)

include(GNUInstallDirs)
