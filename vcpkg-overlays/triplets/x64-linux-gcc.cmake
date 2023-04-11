set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)

#set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
#set(LINK_FLAGS "-static-libgcc -static-libstdc++ -static")
#set(ENV{LDFLAGS} -static-libgcc -static-libstdc++ -static)

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