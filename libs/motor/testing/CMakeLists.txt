if (BUILD_EXAMPLES)
  add_subdirectory(example)
endif ()
if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

add_subdirectory(mocks)
