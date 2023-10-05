vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO skaginn3x/framework
  REF ${VERSION}
  SHA512 20478778c26e086b28b48d97839f28d949ea1cc80df1c30918d00e9de7d86bd434058a6ff098eca76dec88c56426a001daa2782f3e188b7162472b09fc1a0f64
)

if ("build-exes" IN_LIST FEATURES)
  set(BUILD_EXES ON)
else()
  set(BUILD_EXES OFF)
endif()

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
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
