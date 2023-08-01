 cmake_minimum_required(VERSION 3.24)
 project(mqtt-sparkplug)

 find_package(protobuf CONFIG REQUIRED)

 # ----------------------------------------------------------
 include(FetchContent)
 FetchContent_Declare(
         sparkplug_proto
         GIT_REPOSITORY https://github.com/eclipse/tahu.git
         GIT_TAG v1.0.4
 )
 FetchContent_MakeAvailable(sparkplug_proto)
 # FetchContent_Populate(sparkplug_proto)

 message(STATUS "---------------------------------------------------")
 get_cmake_property(_variableNames VARIABLES)
 list (SORT _variableNames)
 foreach (_variableName ${_variableNames})
  if (_variableName MATCHES "sparkplug_proto")
   message(STATUS "${_variableName}=${${_variableName}}")
  endif()
 endforeach()
 message(STATUS "---------------------------------------------------")

#  if (EXISTS ${sparkplug_proto_SOURCE_DIR})
#      message(STATUS "Found sparkplug file")
#  else ()
#      message(STATUS "Could not find sparkplug file")
#  endif ()
#
#  if (EXISTS ${sparkplug_proto_BINARY_DIR})
#   message(STATUS "Found sparkplug file - binary")
#  else ()
#   message(STATUS "Could not find sparkplug file - binary")
#  endif ()

 # ----------------------------------------------------------

 file(
         COPY
         ${sparkplug_proto_SOURCE_DIR}/sparkplug_b/sparkplug_b.proto
         DESTINATION
         ${CMAKE_CURRENT_SOURCE_DIR}/sparkplug_b/
 )

 add_library(sparkplug_proto
         ${CMAKE_CURRENT_SOURCE_DIR}/sparkplug_b/sparkplug_b.proto
         )

 # It looks like the code protobuf generates
 # generates some warnings inside clang
 # These warnings are also in headers so we can't
 # have these options PRIVATE
 if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
     target_compile_options(sparkplug_proto
             PUBLIC
             -Wno-reserved-identifier
             -Wno-switch-enum
             -Wno-sign-conversion
             -Wno-shorten-64-to-32
             -Wno-unsafe-buffer-usage
             -Wno-missing-prototypes
             -Wno-global-constructors
             )
 endif ()

 add_library(tfc::sparkplug::proto ALIAS sparkplug_proto)
 target_link_libraries(sparkplug_proto
         PUBLIC
         protobuf::libprotobuf
         )
 target_include_directories(sparkplug_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
 protobuf_generate(TARGET sparkplug_proto LANGUAGE cpp)

 add_subdirectory(example)

