
add_executable(libsnitch_example libsnitch_example.cpp)

target_link_libraries(libsnitch_example
  PRIVATE
    tfc::base
    tfc::logger
    tfc::operation_mode
)
