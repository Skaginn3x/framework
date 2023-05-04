include(FeatureSummary)

# Use option BUILD_TESTING to disable tests

set(TFC_DBUS_DOMAIN "com" CACHE STRING "D-Bus domain, for example 'com' or 'org'")
add_feature_info("TFC_DBUS_DOMAIN" TFC_DBUS_DOMAIN "D-Bus domain, for example 'com' or 'org',
  Current value: '${TFC_DBUS_DOMAIN}'")

set(TFC_DBUS_ORGANIZATION "skaginn3x" CACHE STRING "D-Bus organization, for example 'freedesktop'")
add_feature_info("TFC_DBUS_ORGANIZATION" TFC_DBUS_ORGANIZATION "D-Bus organization, for example 'freedesktop'.
  Current value: '${TFC_DBUS_ORGANIZATION}'")

option(BUILD_DOCS "Indicates whether documentation should be built." OFF)
add_feature_info("BUILD_DOCS" BUILD_DOCS "Indicates whether documentation should be built.")

option(ENABLE_CODE_COVERAGE_INSTRUMENTATION "Enable code instrumentation" OFF)
add_feature_info("ENABLE_CODE_COVERAGE_INSTRUMENTATION" ENABLE_CODE_COVERAGE_INSTRUMENTATION
    "Enable code instrumentation to allow generating code coverage after running tests")
if(ENABLE_CODE_COVERAGE_INSTRUMENTATION)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    #https://gcc.gnu.org/onlinedocs/gcc-12.1.0/gcc/Instrumentation-Options.html
    add_compile_options(--coverage)
    add_link_options(--coverage)
  endif()
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # https://clang.llvm.org/docs/SourceBasedCodeCoverage.html
    # for generating reports from generated coverage data
    # take a look at https://clang.llvm.org/docs/SourceBasedCodeCoverage.html#creating-coverage-reports
    add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
    add_link_options(-fprofile-instr-generate -fcoverage-mapping)
  endif()
endif(ENABLE_CODE_COVERAGE_INSTRUMENTATION)


option(ENABLE_SANITIZATION "Enables google sanitizer mode" OFF)
add_feature_info("ENABLE_SANITIZATION" ENABLE_SANITIZATION
    "Enables address, thread and leak sanitizing on code")
if(ENABLE_SANITIZATION)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(
        -fsanitize=address
        -fsanitize=leak
        -fsanitize-address-use-after-scope
        -fsanitize=undefined
        -fsanitize=integer
        -fsanitize=nullability
        -fsanitize=float-divide-by-zero
        -fsanitize=unsigned-integer-overflow
        -fsanitize=implicit-conversion
        -fsanitize=local-bounds
        -fsanitize=implicit-integer-truncation
        -fsanitize=implicit-integer-arithmetic-value-change
        -fsanitize=implicit-conversion
        -static-libsan
        -fPIC
        -fno-omit-frame-pointer
        -g )
    add_link_options( -lasan -lubsan -ltsan)
  endif()
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(
        -fsanitize=address
        -fsanitize-address-use-after-scope
        -fsanitize=leak
        -fsanitize=undefined
        -static-libasan
        -fPIC
        -fno-omit-frame-pointer
    )
    add_link_options( -lasan -lubsan)
  endif()
endif(ENABLE_SANITIZATION)


if (CMAKE_BUILD_TYPE STREQUAL "Release")
  option(ENABLE_DEBUG_SYMBOLS_IN_RELEASE "Compile with debug symbols for release build" OFF)
  add_feature_info("ENABLE_DEBUG_SYMBOLS_IN_RELEASE" ENABLE_DEBUG_SYMBOLS_IN_RELEASE
      "Compile with debug symbols for release build")
  if (ENABLE_DEBUG_SYMBOLS_IN_RELEASE)
    add_compile_options(-g)
  endif ()
endif ()


option(ENABLE_STATIC_LINKING "Globally add -static link flag to all targets" OFF)
add_feature_info("ENABLE_STATIC_LINKING" ENABLE_STATIC_LINKING
    "Globally add -static link flag to all targets")
