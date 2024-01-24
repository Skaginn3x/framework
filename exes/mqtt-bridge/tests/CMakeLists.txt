add_executable(test_mqtt_bridge
  ../src/client.cpp
  ../src/spark_plug_interface.cpp
  ../src/external_to_tfc.cpp
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
    tfc::base
    tfc::logger
    tfc::stx
    tfc::ipc
    tfc::operation_mode
    tfc::confman
    Boost::boost
    Boost::log
    Boost::program_options
    OpenSSL::SSL OpenSSL::Crypto
    tfc::sparkplug::proto
)

target_compile_definitions(test_mqtt_bridge
  PUBLIC
    ASYNC_MQTT_USE_TLS
)

find_package(async_mqtt_iface CONFIG REQUIRED)

find_package(Boost REQUIRED COMPONENTS unit_test_framework)

add_executable(integration_tests
        ../src/client.cpp
        ../src/spark_plug_interface.cpp
        ../src/external_to_tfc.cpp
        ../src/signal_names.cpp
        src/integration_tests.cpp
        )

target_include_directories(integration_tests
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        )

target_link_libraries(integration_tests
        PUBLIC
        tfc::base
        tfc::logger
        tfc::stx
        tfc::ipc
        tfc::operation_mode
        tfc::confman
        Boost::boost
        Boost::log
        Boost::program_options
        Boost::unit_test_framework
        OpenSSL::SSL OpenSSL::Crypto
        tfc::sparkplug::proto
        async_mqtt_iface::async_mqtt_iface
        )

include(GNUInstallDirs)

add_subdirectory(tool)
