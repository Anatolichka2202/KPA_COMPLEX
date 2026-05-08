add_library(bkd_network STATIC
    # пока только заглушка

    inetwork_layer.h
    emulated_network.h emulated_network.cpp
    dummy_network.h
    real_yls_network.h
    real_yls_network.cpp
    yls_emulator.h yls_emulator.cpp
    support_yls.h support_yls.cpp

)

target_include_directories(bkd_network PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(bkd_network PRIVATE Boost::boost bkd_core)
target_link_libraries(bkd_network PRIVATE ws2_32)
