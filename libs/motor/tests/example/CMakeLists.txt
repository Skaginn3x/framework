
add_executable(motor_example motor_example.cxx)

target_link_libraries(motor_example
    tfc::base
    tfc::logger
    tfc::motor
)
