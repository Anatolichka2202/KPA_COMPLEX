add_library(bkd_network STATIC
    inetwork_layer.h
    real_yls_network.h real_yls_network.cpp
    i_proxy_backend.h
    proxy_core.h proxy_core.cpp
    windivert_proxy_backend.h windivert_proxy_backend.cpp
)

target_include_directories(bkd_network PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(bkd_network PRIVATE Boost::boost bkd_core ws2_32)

# Туннель уже добавлен в корневом CMakeLists.txt, просто линкуемся
target_link_libraries(bkd_network PRIVATE tunnel)
