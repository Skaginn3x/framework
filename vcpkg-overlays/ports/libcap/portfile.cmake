vcpkg_from_git(
  OUT_SOURCE_PATH SOURCE_PATH
  URL git://git.kernel.org/pub/scm/libs/libcap/libcap.git
  FETCH_REF "libcap-${VERSION}"
  REF 3c7dda330bd9a154bb5b878d31fd591e4951fe17
  PATCHES
    configure.patch
)

# skip_configure is broken https://github.com/microsoft/vcpkg/issues/14389
#vcpkg_configure_make(
#  SOURCE_PATH "${SOURCE_PATH}"
#  SKIP_CONFIGURE
#)
foreach(BUILDTYPE "debug" "release")
  if(BUILDTYPE STREQUAL "debug")
    set(SHORT_BUILDTYPE "-dbg")
    set(CMAKE_BUILDTYPE "DEBUG")
    set(PATH_SUFFIX "/debug")
  else()
    if (_VCPKG_NO_DEBUG)
      set(SHORT_BUILDTYPE "")
    else()
      set(SHORT_BUILDTYPE "-rel")
    endif()
    set(CMAKE_BUILDTYPE "RELEASE")
    set(PATH_SUFFIX "")
  endif()

  set(WORKING_DIRECTORY "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}${SHORT_BUILDTYPE}")

  if (NOT EXISTS "${WORKING_DIRECTORY}")
    file(MAKE_DIRECTORY "${WORKING_DIRECTORY}")
  endif()

  message(STATUS "Source path: ${SOURCE_PATH}")
  message(STATUS "Destination: ${WORKING_DIRECTORY}")

  file(INSTALL ${SOURCE_PATH}/ DESTINATION ${WORKING_DIRECTORY})
endforeach()

vcpkg_build_make(
)

#vcpkg_cmake_configure(
#    SOURCE_PATH "${SOURCE_PATH}"
#    OPTIONS
#    -DBUILD_DOC=OFF
#    -DBUILD_LIBSYSTEMD=ON
#)
#
#vcpkg_cmake_install()
#
#vcpkg_cmake_config_fixup(PACKAGE_NAME sdbus-c++ CONFIG_PATH lib/cmake/sdbus-c++/)
#vcpkg_fixup_pkgconfig()
#vcpkg_copy_pdbs()
#
#file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
#file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
#file(INSTALL ${SOURCE_PATH}/COPYING DESTINATION ${CURRENT_PACKAGES_DIR}/share/sdbus-cpp RENAME copyright)
