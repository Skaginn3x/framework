 cmake_minimum_required(VERSION 3.21)
 project(mqtt_broadcaster)

 add_executable(mqtt-broadcaster "src/main.cpp")
 add_executable(mqtt-broadcaster-tests "tests/mqtt_broadcaster_tests.cpp")

 find_package(Boost 1.81.0 REQUIRED COMPONENTS log program_options)
 # find_package(unofficial-libuuid CONFIG REQUIRED)
 # find_package(eclipse-paho-mqtt-c CONFIG REQUIRED)

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
         # eclipse-paho-mqtt-c::paho-mqtt3as-static
         # unofficial::UUID::uuid
         )

 set(COMMON_COMPILE_DEFINITIONS PUBLIC ASYNC_MQTT_USE_LOG)

 set(COMMON_INCLUDE_DIRECTORIES
         PUBLIC
         ${CMAKE_CURRENT_SOURCE_DIR}/inc
         )

 foreach (TARGET mqtt-broadcaster mqtt-broadcaster-tests)
     target_compile_definitions(${TARGET} ${COMMON_COMPILE_DEFINITIONS})
     target_link_libraries(${TARGET} PUBLIC ${COMMON_LIBS})
     target_include_directories(${TARGET} ${COMMON_INCLUDE_DIRECTORIES})
 endforeach ()

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
