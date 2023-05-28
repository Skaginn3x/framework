
add_executable(confman_stubs_example main.cpp)

target_link_libraries(confman_stubs_example
  PUBLIC
    tfc::stubs::confman
)
