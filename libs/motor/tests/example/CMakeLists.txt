
add_executable(motor_example motor_example.cpp)

target_link_libraries(motor_example
  PRIVATE
    tfc::base
    tfc::logger
    tfc::motor
)
