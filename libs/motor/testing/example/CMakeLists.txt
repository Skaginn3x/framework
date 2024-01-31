
add_executable(motor_example motor_example.cxx)

target_link_libraries(motor_example
    tfc::base
    tfc::logger
    tfc::motor
)

add_executable(positioner_example positioner_example.cpp)

target_link_libraries(positioner_example
  tfc::base
  tfc::logger
  tfc::motor
)
