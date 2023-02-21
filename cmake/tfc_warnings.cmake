
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
endif()
