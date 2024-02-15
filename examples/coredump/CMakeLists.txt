add_executable(example_segfault example_segfault.cpp segfault.cpp)
target_link_libraries(example_segfault
  PRIVATE
    tfc::base
)

include(tfc_split_debug_info)
tfc_split_debug_info(example_segfault)
