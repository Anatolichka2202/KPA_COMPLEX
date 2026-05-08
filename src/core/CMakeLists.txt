add_library(bkd_core STATIC
    master.cpp
    timer.h timer.cpp
    types.h
    #test_widget.h test_widget.cpp
)

target_include_directories(bkd_core PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(bkd_core PUBLIC  Boost::boost Qt6::Core Qt6::Widgets Qt6::Charts)
target_compile_definitions(bkd_core PRIVATE $<TARGET_PROPERTY:Boost::boost,INTERFACE_COMPILE_DEFINITIONS>)
