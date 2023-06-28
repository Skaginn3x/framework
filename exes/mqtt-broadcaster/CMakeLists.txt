 cmake_minimum_required(VERSION 3.21)
 project(mqtt_broadcaster)

 add_executable(mqtt-broadcaster "src/main.cpp")
 add_executable(mqtt-broadcaster-tests "tests/mqtt_broadcaster_tests.cpp")
 add_executable(minimum-example "src/minimum_example.cpp")
 add_executable(paho "src/paho.cpp")

 # find packages
 find_package(Boost 1.81.0 REQUIRED COMPONENTS log program_options)
 # find_package(async_mqtt_iface CONFIG REQUIRED)

 # a bit hacky but works for now
 list(APPEND CMAKE_PREFIX_PATH "/opt/vcpkg/packages/libuuid_x64-linux/share/unofficial-libuuid")
 find_package(unofficial-libuuid CONFIG REQUIRED)

 find_package(eclipse-paho-mqtt-c CONFIG REQUIRED)

 list(APPEND CMAKE_PREFIX_PATH "/opt/vcpkg/packages/paho-mqttpp3_x64-linux/share/PahoMqttCpp/")
 find_package(PahoMqttCpp CONFIG REQUIRED)

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
         # async_mqtt_iface::async_mqtt_iface
         eclipse-paho-mqtt-c::paho-mqtt3a-static
         eclipse-paho-mqtt-c::paho-mqtt3c-static
         eclipse-paho-mqtt-c::paho-mqtt3as-static
         eclipse-paho-mqtt-c::paho-mqtt3cs-static
         PahoMqttCpp::paho-mqttpp3-static
         uuid
         )

 # set common compile definitions
 set(COMMON_COMPILE_DEFINITIONS PUBLIC ASYNC_MQTT_USE_LOG)

 # set common include directories
 set(COMMON_INCLUDE_DIRECTORIES
         PUBLIC
         ${CMAKE_CURRENT_SOURCE_DIR}/inc
         )

 # target properties for all targets
 foreach (TARGET mqtt-broadcaster mqtt-broadcaster-tests minimum-example paho)
     target_compile_definitions(${TARGET} ${COMMON_COMPILE_DEFINITIONS})
     target_link_libraries(${TARGET} PUBLIC ${COMMON_LIBS})
     target_include_directories(${TARGET} ${COMMON_INCLUDE_DIRECTORIES})
 endforeach ()

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

