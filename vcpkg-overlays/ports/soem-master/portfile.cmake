vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO OpenEtherCATsociety/SOEM
  REF a901500618405760a564e64a6816705e29f50f9f
  SHA512 d554bc1c3780b1a81402a7fda490f516caba6bd943a28482740b5c9d97e4273a11546e79c92796487ee9901f568cbf1b329d4e1c1d32602fdce0088a77c82443
  HEAD_REF master
)

vcpkg_cmake_configure(
  SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()

file(REMOVE_RECURSE
  "${CURRENT_PACKAGES_DIR}/bin"
  "${CURRENT_PACKAGES_DIR}/debug/bin"
  "${CURRENT_PACKAGES_DIR}/debug/include"
  "${CURRENT_PACKAGES_DIR}/debug/share"
)

# Handle copyright
file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)