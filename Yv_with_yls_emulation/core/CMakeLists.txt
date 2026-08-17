add_library(yav_core STATIC)

# Источники для core
target_sources(yav_core PRIVATE
    config.cpp
    configmanager.cpp
    logic.cpp
    network.cpp

    data_processor_factory.cpp
    full_emulation_processor.cpp
    network_emulation_processor.cpp
    real_network_processor.cpp
)

# Заголовки для core
target_sources(yav_core PRIVATE
    config.h
    configmanager.h
    logic.h
    network.h
    Protocol.h
    ylsemulator.h
    timer_interface.h
    idata_processor.h
    full_emulation_processor.h
    network_emulation_processor.h
    real_network_processor.h
)

# Директории include
target_include_directories(yav_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Линковка Qt компонентов (core использует только Core и Network)
target_link_libraries(yav_core
    Qt6::Core
    Qt6::Network
)

set_target_properties(yav_core PROPERTIES
    AUTOMOC ON
)
