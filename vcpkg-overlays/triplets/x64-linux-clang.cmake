set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-stdlib=libc++")
set(LINK_FLAGS "-fuse-ld=lld -stdlib=libc++")

set(ENV{CC} clang)
set(ENV{CXX} clang++)
set(ENV{CXXFLAGS} "-stdlib=libc++")
set(ENV{LDFLAGS} "-fuse-ld=lld -stdlib=libc++")
