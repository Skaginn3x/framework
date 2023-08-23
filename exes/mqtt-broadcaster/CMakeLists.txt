add_executable(mqtt-broadcaster
  src/main.cpp
)

find_package(Boost REQUIRED COMPONENTS log program_options)
find_package(OpenSSL REQUIRED)

target_include_directories(mqtt-broadcaster
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

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
    OpenSSL::SSL OpenSSL::Crypto
    tfc::sparkplug::proto
)

target_compile_definitions(mqtt-broadcaster
  PUBLIC
    ASYNC_MQTT_USE_TLS
)

target_compile_options(mqtt-broadcaster PRIVATE -Wno-exit-time-destructors)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

