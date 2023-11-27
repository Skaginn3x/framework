
add_executable(mqtt-sparkplug-example mqtt-sparkplug-example.cpp)

target_link_libraries(mqtt-sparkplug-example PUBLIC tfc::sparkplug::proto)



