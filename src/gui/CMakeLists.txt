add_library(kpa_gui STATIC
    mainwindow.cpp
    mainwindow.ui
    block.cpp
    block.ui
    blockmodel.cpp
    widgets/angle_guard/advancedgauge.cpp
    widgets/angle_guard/gaugewidget.cpp
    widgets/angle_guard/stepslider.cpp
    core/queues.cpp
)

target_include_directories(kpa_gui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/angle_guard
)

target_link_libraries(kpa_gui PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Charts
    bkd_core
)

# Для AUTOUIC добавить пути поиска
set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/angle_guard
)
