project(ec)
cmake_minimum_required(VERSION 3.21)

find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")
find_path(ETHERCAT_INCLUDE_DIRS "ethercat/ecrt.h")
find_path(SOEM_INCLUDE_DIRS "soem/ethercat.h")
find_package(fmt CONFIG REQUIRED)


add_library(ec STATIC src/ec.cpp)
add_library(tfc::ec ALIAS ec)

target_include_directories(ec
  PUBLIC
    inc/public
  ${BEXT_SML_INCLUDE_DIRS}
  ${SOEM_INCLUDE_DIRS}/soem
  PRIVATE
    inc/
)
target_link_directories(ec PUBLIC /home/omar/source/repos/framework/vcpkg-sysroot-gcc-libstdcxx-dynamic/x64-linux-gcc-dynamic/lib)

target_link_libraries(ec
  PUBLIC
    libsoem.a
  PRIVATE
    fmt::fmt
  )

#add_library_to_docs(tfc::ec)

add_subdirectory(tests)