
add_executable(liboperation_mode_example liboperation_mode_example.cpp)

target_link_libraries(liboperation_mode_example
  PRIVATE
    tfc::base
    tfc::logger
    tfc::operation_mode
)
