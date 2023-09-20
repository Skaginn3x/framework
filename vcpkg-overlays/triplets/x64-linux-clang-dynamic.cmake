set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)

find_program(CLANG-17 "clang-17")
if(CLANG-17)
  set(CMAKE_C_COMPILER clang-17)
  set(CMAKE_CXX_COMPILER clang++-17)
  set(CMAKE_CXX_FLAGS "-stdlib=libc++")
  set(LINK_FLAGS "-fuse-ld=lld")

  set(ENV{CC} clang-17)
  set(ENV{CXX} clang++-17)
  set(ENV{CXXFLAGS} "-stdlib=libc++")
  set(ENV{LDFLAGS} "-fuse-ld=lld")
else()
  set(CMAKE_C_COMPILER /opt/clang-17.0.1/bin/clang)
  set(CMAKE_CXX_COMPILER /opt/clang-17.0.1/bin/clang++)
  set(CMAKE_CXX_FLAGS "-stdlib=libc++ -I/opt/clang-17.0.1/include/ -I/opt/clang-17.0.1/include/x86_64-unknown-linux-gnu/c++/v1")
  set(LINK_FLAGS "-fuse-ld=lld")

  set(ENV{CC} /opt/clang-17.0.1/bin/clang)
  set(ENV{CXX} /opt/clang-17.0.1/bin/clang++)
  set(ENV{CXXFLAGS} "-stdlib=libc++ -I/opt/clang-17.0.1/include/ -I/opt/clang-17.0.1/include/x86_64-unknown-linux-gnu/c++/v1")
  set(ENV{LDFLAGS} "-fuse-ld=lld")
endif ()

set(VCPKG_FIXUP_ELF_RPATH ON)
