
add_executable(testme main.cpp)
add_subdirectory(connection)
find_package(Boost REQUIRED)

target_link_libraries(testme
  PUBLIC
    Boost::boost
    testme_connection
)
