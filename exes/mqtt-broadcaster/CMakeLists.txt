project(mqtt_broadcaster)

cmake_minimum_required(VERSION 3.21)

add_executable(mqtt-broadcaster "src/main.cpp")

add_executable(mqtt-broadcaster-tests "tests/mqtt_broadcaster_tests.cpp")

find_package(Boost REQUIRED)

find_package(async_mqtt_iface CONFIG REQUIRED)

find_package(Boost REQUIRED COMPONENTS program_options)

find_package(Boost 1.81.0 REQUIRED COMPONENTS log)
target_compile_definitions(mqtt-broadcaster PUBLIC ASYNC_MQTT_USE_LOG)
target_link_libraries(mqtt-broadcaster
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
        async_mqtt_iface::async_mqtt_iface
        )


target_compile_definitions(mqtt-broadcaster-tests PUBLIC ASYNC_MQTT_USE_LOG)
target_link_libraries(mqtt-broadcaster-tests
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
        async_mqtt_iface::async_mqtt_iface
        )

target_include_directories(mqtt-broadcaster
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        )


target_include_directories(mqtt-broadcaster-tests
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        )