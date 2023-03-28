vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO stephenberry/glaze
    REF c89939557338dfe6ecee159773b83224c448cd3c
    SHA512 c01ad5ecfe4a2a621b303416ad068c30378d58e05b61828b8d56101d36a6611beafb0d65c8cde46be2e696160ba785afd77a2356adbdc27bccc8ed0d2ec07b26
    PATCHES
      disable-dev-mode.patch
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")

file(INSTALL ${SOURCE_PATH}/LICENSE.txt DESTINATION ${CURRENT_PACKAGES_DIR}/share/glaze RENAME copyright)
