add_library(testme_connection connection.cpp)

find_package(Boost REQUIRED)
#find_package(unofficial-sdbusplus CONFIG REQUIRED)
find_package(unofficial-systemd CONFIG REQUIRED)
target_link_libraries(testme_connection
  PRIVATE
    Boost::boost
#    unofficial::sdbusplus
    unofficial::systemd::libsystemd
    systemd # todo why
)

target_include_directories(testme_connection
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
