if(VCPKG_TARGET_IS_LINUX)
    message("Note: `mp-units` requires Clang16+ or GCC11+")
endif()

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO JohelEGP/units-1
    REF "modules"
    SHA512 3ce35d8d19e12da70923a09b48be0629c543f5316fb4a6391fa142b9432a44d22ceebbbfbc6b1470cade71f47b8bee19a92de975557ab5d44ac85622f6c4931a
)

set(USE_LIBFMT OFF)
if ("use-libfmt" IN_LIST FEATURES)
    set(USE_LIBFMT ON)
endif()

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}/src"
    OPTIONS
      -DMP_UNITS_USE_LIBFMT=ON
      -DMP_UNITS_BUILD_MODULES=ON
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH "lib/cmake/${PORT}")

# Handle copyright/readme/package files
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.md")
file(INSTALL "${SOURCE_PATH}/README.md" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug"
                    "${CURRENT_PACKAGES_DIR}/lib") # Header only
