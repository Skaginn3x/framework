
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(
    -Weverything
    -Werror
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-pre-c++14-compat
    -Wno-pre-c++17-compat
    -Wno-pre-c++20-compat-pedantic
    -Wno-c++20-compat
    -Wno-unused-parameter
    -Wno-padded
    -Wno-unused-command-line-argument
  )
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(
    -Wall
    -Wextra
    -Werror
  )
  # Removing NDEBUG removes asserts that check nullness of pointers
  # this causes gcc to determine that potential nullptr dereferences
  # are in place where there are none. Disable this warning for now.
  if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(
      -Wno-error=null-dereference
      -Wno-error=maybe-uninitialized
    )
  endif ()
endif()
