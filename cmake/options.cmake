
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


option(ENABLE_ADDRESS_SANITIZATION "Enables clang address sanitizer mode" OFF)
add_feature_info("ENABLE_ADDRESS_SANITIZATION" ENABLE_ADDRESS_SANITIZATION
    "Enables address sanitizing on code")
if(ENABLE_ADDRESS_SANITIZATION)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(
        -fsanitize=address
        -fsanitize=leak
        -fsanitize=thread
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
    add_link_options( -lasan -lubsan)
  endif()
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(
        -fsanitize=address
        -fsanitize-address-use-after-scope
        -fsanitize=leak
        -fsanitize=thread
        -fsanitize=undefined
        -static-libasan
        -fPIC
        -fno-omit-frame-pointer
    )
    add_link_options( -lasan -lubsan)
  endif()
endif(ENABLE_ADDRESS_SANITIZATION)


