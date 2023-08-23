add_executable(mqtt-broadcaster-unit-tests
  unit_tests.cpp
)

target_include_directories(mqtt-broadcaster-unit-tests
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(mqtt-broadcaster-unit-tests
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

target_compile_options(mqtt-broadcaster-unit-tests PRIVATE -Wno-exit-time-destructors)

target_compile_definitions(mqtt-broadcaster-unit-tests
  PUBLIC
    ASYNC_MQTT_USE_TLS
)

target_compile_options(mqtt-broadcaster-unit-tests PRIVATE -g)