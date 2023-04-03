set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)

set(CMAKE_C_COMPILER /opt/clang-16.0.0/bin/clang)
set(CMAKE_CXX_COMPILER /opt/clang-16.0.0/bin/clang++)
set(CMAKE_CXX_FLAGS "-stdlib=libc++ -I/opt/clang-16.0.0/include/ -I/opt/clang-16.0.0/include/x86_64-unknown-linux-gnu/c++/v1")
set(LINK_FLAGS "-fuse-ld=lld")

set(ENV{CC} /opt/clang-16.0.0/bin/clang)
set(ENV{CXX} /opt/clang-16.0.0/bin/clang++)
set(ENV{CXXFLAGS} "-stdlib=libc++ -I/opt/clang-16.0.0/include/ -I/opt/clang-16.0.0/include/x86_64-unknown-linux-gnu/c++/v1")
set(ENV{LDFLAGS} "-fuse-ld=lld")
