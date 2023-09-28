

add_executable(test_rig main.cpp)
target_link_libraries(test_rig
  PRIVATE
    tfc::base
    tfc::confman
    tfc::ipc
    tfc::logger
    tfc::operation_mode
)
