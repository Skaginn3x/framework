vcpkg_from_git(
  OUT_SOURCE_PATH SOURCE_PATH
  URL git://git.kernel.org/pub/scm/libs/libcap/libcap.git
  FETCH_REF "libcap-${VERSION}"
  REF 3c7dda330bd9a154bb5b878d31fd591e4951fe17
  PATCHES
    configure.patch
)

# skip_configure is broken https://github.com/microsoft/vcpkg/issues/14389
# so we use patch to create empty configure executable

vcpkg_configure_make(
  SOURCE_PATH "${SOURCE_PATH}"
  COPY_SOURCE
)

vcpkg_build_make(SUBPATH libcap
    OPTIONS
      prefix=${CURRENT_INSTALLED_DIR}
)

file(INSTALL ${SOURCE_PATH}/libcap/include/ DESTINATION ${CURRENT_PACKAGES_DIR}/include FILES_MATCHING PATTERN "*.h")

set(BUILD_DIR_RELEASE "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel")
message(STATUS "Custom libcap INSTALL to ${BUILD_DIR_RELEASE}")
if(VCPKG_LIBRARY_LINKAGE STREQUAL "static")
  file(INSTALL ${BUILD_DIR_RELEASE}/libcap/libcap.a DESTINATION ${CURRENT_PACKAGES_DIR}/lib )
  file(INSTALL ${BUILD_DIR_RELEASE}/libcap/libpsx.a DESTINATION ${CURRENT_PACKAGES_DIR}/lib )
else()
  file(INSTALL ${BUILD_DIR_RELEASE}/libcap/libcap.so DESTINATION ${CURRENT_PACKAGES_DIR}/lib FOLLOW_SYMLINK_CHAIN)
  file(INSTALL ${BUILD_DIR_RELEASE}/libcap/libpsx.so DESTINATION ${CURRENT_PACKAGES_DIR}/lib FOLLOW_SYMLINK_CHAIN)
endif()

set(BUILD_DIR_DEBUG "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg")
message(STATUS "Custom libcap INSTALL to ${BUILD_DIR_DEBUG}")
if(VCPKG_LIBRARY_LINKAGE STREQUAL "static")
  file(INSTALL ${BUILD_DIR_DEBUG}/libcap/libcap.a DESTINATION ${CURRENT_PACKAGES_DIR}/debug/lib )
  file(INSTALL ${BUILD_DIR_DEBUG}/libcap/libpsx.a DESTINATION ${CURRENT_PACKAGES_DIR}/debug/lib )
else()
  file(INSTALL ${BUILD_DIR_DEBUG}/libcap/libcap.so DESTINATION ${CURRENT_PACKAGES_DIR}/debug/lib FOLLOW_SYMLINK_CHAIN)
  file(INSTALL ${BUILD_DIR_DEBUG}/libcap/libpsx.so DESTINATION ${CURRENT_PACKAGES_DIR}/debug/lib FOLLOW_SYMLINK_CHAIN)
endif()

file(INSTALL ${BUILD_DIR_RELEASE}/libcap/libcap.pc DESTINATION ${CURRENT_PACKAGES_DIR}/lib/pkgconfig )
file(INSTALL ${BUILD_DIR_RELEASE}/libcap/libpsx.pc DESTINATION ${CURRENT_PACKAGES_DIR}/lib/pkgconfig )
file(INSTALL ${BUILD_DIR_DEBUG}/libcap/libcap.pc DESTINATION ${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig )
file(INSTALL ${BUILD_DIR_DEBUG}/libcap/libpsx.pc DESTINATION ${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig )

vcpkg_install_copyright(FILE_LIST ${SOURCE_PATH}/License)

vcpkg_fixup_pkgconfig()
