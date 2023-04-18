# this file is duplicate of x64-linux-gcc
# forcing vcpkg to generate different cache key
# so vcpkg cache won't be used between muslc and glibc
set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)

find_program(GCC-12 "gcc-12")
if(GCC-12)
  set(CMAKE_C_COMPILER gcc-12)
  set(CMAKE_CXX_COMPILER g++-12)

  set(ENV{CC} gcc-12)
  set(ENV{CXX} g++-12)
else()
  set(CMAKE_C_COMPILER gcc)
  set(CMAKE_CXX_COMPILER g++)

  set(ENV{CC} gcc)
  set(ENV{CXX} g++)
endif ()