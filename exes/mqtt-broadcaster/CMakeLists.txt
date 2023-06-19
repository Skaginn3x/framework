 add_executable(mqtt-broadcaster "src/main.cpp")
 add_executable(mqtt-broadcaster-tests "tests/mqtt_broadcaster_tests.cpp")
 add_executable(sample "src/sample.cpp")

 find_package(Boost REQUIRED COMPONENTS log program_options)

 find_package(OpenSSL REQUIRED)

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
         OpenSSL::SSL OpenSSL::Crypto
         )

 target_compile_definitions(mqtt-broadcaster
         PUBLIC
         ASYNC_MQTT_USE_LOG
         ASYNC_MQTT_USE_TLS
         )

 target_compile_options(mqtt-broadcaster PRIVATE -g)

 # ------------------------------------------------------------

 target_include_directories(sample
         PUBLIC
         ${CMAKE_CURRENT_SOURCE_DIR}/inc
         )

 target_link_libraries(sample
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
         )

 target_compile_definitions(sample
         PUBLIC
         ASYNC_MQTT_USE_LOG
         ASYNC_MQTT_USE_TLS
         )

 target_compile_options(sample PRIVATE -g)

 # ------------------------------------------------------------

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
