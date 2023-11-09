add_executable(example_sml_interface example_sml_interface.cpp)

target_link_libraries(example_sml_interface
  PRIVATE
    tfc::logger
    tfc::base
    tfc::dbus_util
)

add_example_to_docs(example_sml_interface)
