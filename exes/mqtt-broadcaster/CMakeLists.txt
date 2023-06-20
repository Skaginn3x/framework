project(mqtt_broadcaster)

cmake_minimum_required(VERSION 3.21)

add_executable(mqtt-broadcaster "src/main.cpp")

find_package(Boost REQUIRED)

find_package(async_mqtt_iface CONFIG REQUIRED)

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
        async_mqtt_iface::async_mqtt_iface
        )


