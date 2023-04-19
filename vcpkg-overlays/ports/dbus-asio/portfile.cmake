vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO dbus-asio/dbus-asio
    REF 9f2cb2d836003a8247fbda9f7f606cd475c48ef1
    SHA512 aca4d1ea58f03b9917e939e82fa154dfbf7d8576a8f2d6b05c13c0128584c2637a53826405626d86101725b1adc83de2354c2365a90ef75f629a322a46883d1a
    PATCHES
      disable-tests.patch
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")

file(INSTALL ${SOURCE_PATH}/COPYING DESTINATION ${CURRENT_PACKAGES_DIR}/share/dbus-asio RENAME copyright)
