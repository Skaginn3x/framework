#project(mqtt_broadcaster)
#
#cmake_minimum_required(VERSION 3.21)
#
#add_executable(mqtt-broadcaster "src/main.cpp")
#add_executable(mqtt-broadcaster-tests "tests/mqtt_broadcaster_tests.cpp")
#add_executable(minimum-example "src/minimum_example.cpp")
#
#find_package(Boost REQUIRED)
#
#find_package(async_mqtt_iface CONFIG REQUIRED)
#
#find_package(Boost REQUIRED COMPONENTS program_options)
#
#find_package(Boost 1.81.0 REQUIRED COMPONENTS log)
#
#target_compile_definitions(mqtt-broadcaster PUBLIC ASYNC_MQTT_USE_LOG)
#target_link_libraries(mqtt-broadcaster
#        PUBLIC
#        tfc::base
#        tfc::logger
#        tfc::stx
#        tfc::ipc
#        tfc::operation_mode
#        tfc::confman
#        Boost::boost
#        Boost::log
#        Boost::program_options
#        async_mqtt_iface::async_mqtt_iface
#        )
#
#
#target_compile_definitions(minimum-example PUBLIC ASYNC_MQTT_USE_LOG)
#target_link_libraries(minimum-example
#        PUBLIC
#        tfc::base
#        tfc::logger
#        tfc::stx
#        tfc::ipc
#        tfc::operation_mode
#        tfc::confman
#        Boost::boost
#        Boost::log
#        Boost::program_options
#        async_mqtt_iface::async_mqtt_iface
#        )
#
#
#target_compile_definitions(mqtt-broadcaster-tests PUBLIC ASYNC_MQTT_USE_LOG)
#target_link_libraries(mqtt-broadcaster-tests
#        PUBLIC
#        tfc::base
#        tfc::logger
#        tfc::stx
#        tfc::ipc
#        tfc::operation_mode
#        tfc::confman
#        Boost::boost
#        Boost::log
#        Boost::program_options
#        async_mqtt_iface::async_mqtt_iface
#        )
#
#target_include_directories(mqtt-broadcaster
#        PUBLIC
#        ${CMAKE_CURRENT_SOURCE_DIR}/inc
#        )
#
#target_include_directories(minimum-example
#        PUBLIC
#        ${CMAKE_CURRENT_SOURCE_DIR}/inc
#        )
#
#target_include_directories(mqtt-broadcaster-tests
#        PUBLIC
#        ${CMAKE_CURRENT_SOURCE_DIR}/inc
#        )
#
#include(GNUInstallDirs)
#install(
#        TARGETS
#        mqtt-broadcaster
#        DESTINATION
#        ${CMAKE_INSTALL_BINDIR}
#        CONFIGURATIONS Release
#)
#
#install(
#        TARGETS
#        mqtt-broadcaster
#        DESTINATION
#        ${CMAKE_INSTALL_BINDIR}/debug/
#        CONFIGURATIONS Debug
#)









cmake_minimum_required(VERSION 3.21)
project(mqtt_broadcaster)

add_executable(mqtt-broadcaster "src/main.cpp")
#add_executable(mqtt-broadcaster-tests "tests/mqtt_broadcaster_tests.cpp")
#add_executable(minimum-example "src/minimum_example.cpp")

# find packages
find_package(Boost 1.81.0 REQUIRED COMPONENTS log program_options)
find_package(async_mqtt_iface CONFIG REQUIRED)

# set common libraries
set(COMMON_LIBS
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

# set common compile definitions
set(COMMON_COMPILE_DEFINITIONS PUBLIC ASYNC_MQTT_USE_LOG)

# set common include directories
set(COMMON_INCLUDE_DIRECTORIES
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        )

# target properties for all targets
foreach(TARGET mqtt-broadcaster )
    target_compile_definitions(${TARGET} ${COMMON_COMPILE_DEFINITIONS})
    target_link_libraries(${TARGET} PUBLIC ${COMMON_LIBS})
    target_include_directories(${TARGET} ${COMMON_INCLUDE_DIRECTORIES})
endforeach()

# install rules
include(GNUInstallDirs)
install(
        TARGETS mqtt-broadcaster
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        CONFIGURATIONS Release
)

install(
        TARGETS mqtt-broadcaster
        DESTINATION ${CMAKE_INSTALL_BINDIR}/debug/
        CONFIGURATIONS Debug
)
