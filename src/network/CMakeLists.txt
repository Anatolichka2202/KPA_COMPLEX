add_library(bkd_network STATIC
    inetwork_layer.h
    real_yls_network.h
    real_yls_network.cpp
)

target_include_directories(bkd_network PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(bkd_network PRIVATE Boost::boost bkd_core)
target_link_libraries(bkd_network PRIVATE ws2_32)
