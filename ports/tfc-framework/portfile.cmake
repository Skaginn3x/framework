vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO skaginn3x/framework
  REF ${VERSION}
  SHA512 13fb1f4a1317b066d63bc741def0d715cbdc314fe0d674a08fe626d0f96a0e75eb8a9b831cb9150d576ad80da34459b44fc31c680a62f1b8e9c84ef3a196fb05
)

if ("build-exes" IN_LIST FEATURES)
  set(BUILD_EXES ON)
else()
  set(BUILD_EXES OFF)
endif()

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
      -DCMAKE_PROJECT_VERSION=${VERSION} # todo I don't like this
      -DBUILD_EXES=${BUILD_EXES}
      -DBUILD_TESTING=OFF
      -DBUILD_DOCS=OFF
      -DBUILD_EXAMPLES=OFF
)

if (${BUILD_EXES})
  vcpkg_cmake_install(ADD_BIN_TO_PATH)
else ()
  vcpkg_cmake_install()
endif ()

vcpkg_cmake_config_fixup(PACKAGE_NAME tfc)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
