vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO stephenberry/glaze
    REF 9447be42d4537f7ab2b17a0d2d9577620825380a
    SHA512 640b413e18b08cc054072f349c3be9fb729876ee572f676336bd87f14976dc2fc0b965fc854f07eb5de1e84a77fa7593ca0803edcaed4b63621fd4f034f8c6fd
    PATCHES
      disable-dev-mode.patch
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")

file(INSTALL ${SOURCE_PATH}/LICENSE.txt DESTINATION ${CURRENT_PACKAGES_DIR}/share/glaze RENAME copyright)
