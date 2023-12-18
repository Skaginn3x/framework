add_executable(mass_example main.cpp)

find_package(mp-units CONFIG REQUIRED)

target_link_libraries(mass_example
  PRIVATE
    tfc::base
    tfc::ipc
    mp-units::si
)
