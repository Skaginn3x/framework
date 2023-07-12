cmake_minimum_required(VERSION 3.24)
project(mqtt-sparkplug)

find_package(protobuf CONFIG REQUIRED)

include(FetchContent)
FetchContent_Declare(
    sparkplug_proto
    GIT_REPOSITORY https://github.com/eclipse/tahu.git
    GIT_TAG        v1.0.4
)
FetchContent_Populate(sparkplug_proto)

file(
  COPY
    ${sparkplug_proto_SOURCE_DIR}/sparkplug_b/sparkplug_b.proto
  DESTINATION
    ${CMAKE_CURRENT_SOURCE_DIR}/sparkplug_b/
)

add_library(sparkplug_proto
  ${CMAKE_CURRENT_SOURCE_DIR}/sparkplug_b/sparkplug_b.proto
)
add_library(tfc::sparkplug::proto ALIAS sparkplug_proto)
target_link_libraries(sparkplug_proto
  PUBLIC
    protobuf::libprotobuf
)
target_include_directories(sparkplug_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
protobuf_generate(TARGET sparkplug_proto LANGUAGE cpp)

add_subdirectory(example)
