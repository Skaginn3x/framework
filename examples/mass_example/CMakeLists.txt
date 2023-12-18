add_executable(mass_exmple main.cpp)
target_link_libraries(mass_exmple
  PRIVATE
    tfc::base
    tfc::ipc
)
