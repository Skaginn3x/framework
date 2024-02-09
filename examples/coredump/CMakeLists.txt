add_executable(example_throw example_throw.cpp)
target_link_libraries(example_throw
  PRIVATE
    tfc::base
)

include(tfc_split_debug_info)
tfc_split_debug_info(example_throw)
