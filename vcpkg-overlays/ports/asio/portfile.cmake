#header-only library

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO chriskohlhoff/asio
    REF master
    SHA512 bdd7fed2e94bbb69f25e7443df9ea7bc1c1803933c751e015b0e4886c1374130a9d96ed7993ceaee70d43c6e654cb10887fe8b2d0e79f1266f291427362e91cb
    HEAD_REF master
)

# Always use "ASIO_STANDALONE" to avoid boost dependency
vcpkg_replace_string("${SOURCE_PATH}/asio/include/asio/detail/config.hpp" "defined(ASIO_STANDALONE)" "!defined(VCPKG_DISABLE_ASIO_STANDALONE)")

# CMake install
file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)
vcpkg_cmake_install()

vcpkg_cmake_config_fixup()
file(INSTALL
    "${CMAKE_CURRENT_LIST_DIR}/asio-config.cmake"
    DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
)
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")

# Handle copyright
file(INSTALL "${SOURCE_PATH}/asio/LICENSE_1_0.txt" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
