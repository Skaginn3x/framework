 add_executable(mqtt-broadcaster "src/main.cpp")
 add_executable(mqtt-broadcaster-tests "tests/mqtt_broadcaster_tests.cpp")

 find_package(Boost 1.81.0 REQUIRED COMPONENTS log program_options)

 target_include_directories(mqtt-broadcaster
         PUBLIC
         ${CMAKE_CURRENT_SOURCE_DIR}/inc
         )

 target_link_libraries(mqtt-broadcaster
         tfc::base
         tfc::logger
         tfc::stx
         tfc::ipc
         tfc::operation_mode
         tfc::confman
         Boost::boost
         Boost::log
         Boost::program_options
 )

 target_compile_definitions(mqtt-broadcaster
         PUBLIC
         ASYNC_MQTT_USE_LOG
 )

 target_include_directories(mqtt-broadcaster-tests
         PUBLIC
         ${CMAKE_CURRENT_SOURCE_DIR}/inc
         )

 target_link_libraries(mqtt-broadcaster-tests
         tfc::base
         tfc::logger
         tfc::stx
         tfc::ipc
         tfc::operation_mode
         tfc::confman
         Boost::boost
         Boost::log
         Boost::program_options
         )

 target_compile_definitions(mqtt-broadcaster-tests
         PUBLIC
         ASYNC_MQTT_USE_LOG
         )

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
