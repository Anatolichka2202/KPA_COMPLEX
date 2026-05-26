add_library(kpa_gui STATIC
    mainwindow.cpp
    mainwindow.ui
    block.cpp
    block.ui
    blockmodel.cpp
    widgets/angle_guard/advancedgauge.cpp
    widgets/angle_guard/gaugewidget.cpp
    widgets/angle_guard/stepslider.cpp
)

# 1. Эти пути нужны только для компиляции самой библиотеки, они не должны "светиться" наружу.
target_include_directories(kpa_gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/angle_guard
    ${CMAKE_CURRENT_BINARY_DIR} # <--- ВАЖНО: приватно добавляем путь к autogen для самой библиотеки
)

# 2. Для внешних пользователей библиотеки (вашего main.cpp) добавляем папку с ui-файлами.
target_include_directories(kpa_gui INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR} # Нужно, чтобы внешние проекты видели ваши mainwindow.h и block.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/angle_guard          # для advancedgauge.h
    ${CMAKE_CURRENT_BINARY_DIR}/kpa_gui_autogen/include # Здесь мы явно передаем наружу путь к сгенерированным ui-файлам
)

target_link_libraries(kpa_gui PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Charts
    bkd_core
)

set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/angle_guard
)
