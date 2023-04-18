#
# CMake Toolchain file for crosscompiling on ARM.
#
# Target operating system name.
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_CROSSCOMPILING TRUE)

# Name of C compiler.
find_program(AARCH64-GCC "aarch64-linux-gnu-gcc")
if (AARCH64-GCC)
set(CMAKE_C_COMPILER "/usr/bin/aarch64-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/aarch64-linux-gnu-g++")

# Where to look for the target environment. (More paths can be added here)
set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)
set(CMAKE_INCLUDE_PATH  /usr/include/aarch64-linux-gnu)
set(CMAKE_LIBRARY_PATH  /usr/lib/aarch64-linux-gnu)
set(CMAKE_PROGRAM_PATH  /usr/bin/aarch64-linux-gnu)
else ()
set(CMAKE_C_COMPILER "/usr/bin/arm-linux-gnueabi-gcc-12")
set(CMAKE_CXX_COMPILER "/usr/bin/arm-linux-gnueabi-g++-12")

# Where to look for the target environment. (More paths can be added here)
set(CMAKE_FIND_ROOT_PATH /usr/arm-linux-gnueabi)
set(CMAKE_INCLUDE_PATH  /usr/include/arm-linux-gnueabi)
set(CMAKE_LIBRARY_PATH  /usr/lib/arm-linux-gnueabi)
set(CMAKE_PROGRAM_PATH  /usr/bin/arm-linux-gnueabi)
endif ()
# Adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment only.
#set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search headers and libraries in the target environment only.
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)