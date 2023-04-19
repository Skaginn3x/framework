vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO skaginn3x/dbus-asio
    REF vcpkg
    SHA512 6c9df2bf07258a8a059a7f16653701533b606cf8804e14bc7cb416bdeaeb2c54b79b418ca6eea045da3acd1673562098265f4ee37711ade8c68197e41f8348de
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
      -DBUILD_TESTING=OFF
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/dbus-asio)
vcpkg_fixup_pkgconfig()
vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")

file(INSTALL ${SOURCE_PATH}/COPYING DESTINATION ${CURRENT_PACKAGES_DIR}/share/dbus-asio RENAME copyright)
