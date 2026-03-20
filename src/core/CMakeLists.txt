add_library(bkd_core STATIC
    master.cpp
    timer.h timer.cpp
)

target_include_directories(bkd_core PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(bkd_core PUBLIC  Boost::boost)
target_compile_definitions(bkd_core PRIVATE $<TARGET_PROPERTY:Boost::boost,INTERFACE_COMPILE_DEFINITIONS>)
