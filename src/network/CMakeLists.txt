add_library(bkd_network STATIC
    dummy_network.cpp   # пока только заглушка
)

target_include_directories(bkd_network PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(bkd_network PRIVATE Boost::boost bkd_core)
